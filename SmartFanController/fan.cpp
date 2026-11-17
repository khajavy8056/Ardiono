/*
 * ============================================================================
 *   پیاده‌سازی ماژول کنترل فن
 * ============================================================================
 */

#include "fan.h"
#include "filters.h"

// ----------------------------------------------------------------------------
// متغیرهای وضعیت ماژول
// ----------------------------------------------------------------------------
static float  s_targetDuty = 0.0f;    // هدف فعلی (درصد)
static float  s_appliedDuty = 0.0f;   // مقدار اعمال‌شده روی PWM (درصد)
static bool   s_failsafe = false;     // وضعیت حالت ایمن
static uint32_t s_lastUpdateMs = 0UL; // زمان آخرین به‌روزرسانی خروجی PWM

#if FAN_SMOOTHING_ENABLED
static SlewRateLimiter s_slew(FAN_SLEW_RATE_PERCENT_PER_SEC);
#endif

// ----------------------------------------------------------------------------
// پشتیبانی از سنسور سرعت (تاخ) فن چهار سیمه
// ----------------------------------------------------------------------------
#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED

static uint16_t s_rpm = 0;            // آخرین مقدار RPM خوانده‌شده

// شمارنده پول‌ها در وقفه؛ با volatile چون از ISR تغییر می‌کند.
static volatile uint32_t s_tachPulses = 0UL;
static uint32_t s_lastRpmMs = 0UL;

// روتین وقفه: هر پالس روی سیم تاخ
static void tachISR()
{
  s_tachPulses++;
}

#endif

// ----------------------------------------------------------------------------
// تنظیم فرکانس PWM تایمر ۱ برای فن چهار سیمه
// ----------------------------------------------------------------------------
#if (FAN_TYPE == FAN_TYPE_DC_4WIRE)

// کاربرد: فن‌های چهار سیمه ورودی PWM با فرکانس استاندارد ۲۵kHz کار می‌کنند.
// پیاده‌سازی: تایمر ۱ در حالت Fast PWM با TOP در ICR1 (مد ۱۴) و prescaler=1
//   → فرکانس = F_CPU / (1 + ICR1)
// نکته: این کار فرکانس PWM پایه‌های ۹ و ۱۰ را تغییر می‌دهد.
static void configureTimer1Frequency(uint32_t freqHz)
{
  uint32_t top = (F_CPU / freqHz) - 1UL;
  if (top < 1UL)     top = 1UL;
  if (top > 65535UL) top = 65535UL;

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  ICR1   = (uint16_t)top;
  // مد ۱۴ (Fast PWM با TOP=ICR1) + خروجی غیرمعکوس روی کانال‌های A و B
  TCCR1A = (1 << WGM11) | (1 << COM1A1) | (1 << COM1B1);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);
  interrupts();
}

#endif

// ----------------------------------------------------------------------------
// نوشتن مقدار PWM بر اساس درصد سرعت
// ----------------------------------------------------------------------------
// نکته مهم: در فن چهار سیمه تایمر ۱ با TOP=ICR1 کار می‌کند، بنابراین
// نوشتن مستقیم مقدار ۸ بیتی با analogWrite باعث می‌شود Duty Cycle ناقص
// بماند (چون مقیاس TOP برابر ۲۵۵ نیست). به همین دلیل مقدار مقایسه را خودمان
// بر اساس ICR1 محاسبه می‌کنیم. برای فن دو سیمه هم که تایمر پیش‌فرض با
// TOP=255 کار می‌کند، از همان روش یکدست استفاده می‌شود.
static void writeFanPwm(float dutyPercent)
{
#if (FAN_TYPE == FAN_TYPE_DC_4WIRE)
  const uint16_t top = (uint16_t)(ICR1 + 1U);
  const uint16_t compareValue =
      (uint16_t)(((dutyPercent / 100.0f) * (float)top) + 0.5f);

  if (FAN_PWM_PIN == 9) {
    OCR1A = compareValue;   // کانال A تایمر ۱ (پایه ۹)
  } else {
    OCR1B = compareValue;   // کانال B تایمر ۱ (پایه ۱۰)
  }
#else
  const uint8_t pwmValue =
      (uint8_t)(((dutyPercent * 255.0f) / 100.0f) + 0.5f);
  analogWrite(FAN_PWM_PIN, pwmValue);
#endif
}

// ----------------------------------------------------------------------------
void fanBegin()
{
  pinMode(FAN_PWM_PIN, OUTPUT);

#if (FAN_TYPE == FAN_TYPE_DC_4WIRE)
  // تنظیم فرکانس PWM روی تایمر ۱ (فقط پایه‌های ۹ و ۱۰)
  configureTimer1Frequency(FAN4_PWM_FREQUENCY_HZ);
#endif

  writeFanPwm(0.0f);   // در ابتدا فن خاموش

#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED
  // سیم تاخ فن چهار سیمه (خروجی open-collector است؛ اگر میل کشیده نشده
  // باید با یک مقاومت ۱۰kΩ به ۵V پول شود — راهنما در README)
  pinMode(FAN4_TACH_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FAN4_TACH_PIN), tachISR, RISING);
#endif
}

// ----------------------------------------------------------------------------
void fanSetTargetDutyPercent(float dutyPercent)
{
  if (dutyPercent < 0.0f)   dutyPercent = 0.0f;
  if (dutyPercent > 100.0f) dutyPercent = 100.0f;
  s_targetDuty = dutyPercent;
}

// ----------------------------------------------------------------------------
void fanUpdate(uint32_t nowMs)
{
  s_lastUpdateMs = nowMs;

  // در حالت ایمن همیشه سرعت کامل، بدون هیچ هموارسازی (فوری)
  if (s_failsafe) {
    s_appliedDuty = (float)FAILSAFE_DUTY_PERCENT;
  } else {
#if FAN_SMOOTHING_ENABLED
    s_appliedDuty = s_slew.update(s_targetDuty, nowMs);
#else
    s_appliedDuty = s_targetDuty;
#endif
  }

  // اعمال سرعت روی خروجی PWM
  writeFanPwm(s_appliedDuty);

#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED
  // محاسبه RPM هر ۵۰۰ میلی‌ثانیه بر اساس تعداد پول‌ها
  if (nowMs - s_lastRpmMs >= 500UL) {
    const uint32_t dt = nowMs - s_lastRpmMs;
    if (dt > 0UL) {
      // استاندارد فن چهار سیمه: ۲ پالس به ازای هر دور موتور
      s_rpm = (uint16_t)((s_tachPulses * 60000UL) / (2UL * dt));
    }
    s_tachPulses = 0UL;
    s_lastRpmMs = nowMs;
  }
#endif
}

// ----------------------------------------------------------------------------
float fanGetCurrentDutyPercent()
{
  return s_appliedDuty;
}

void fanEnterFailsafe()
{
  s_failsafe = true;
}

void fanExitFailsafe()
{
  s_failsafe = false;
  // از همان مقدار فعلی شروع به حرکت تدریجی می‌کنیم تا جهش سرعت نداشته باشیم
#if FAN_SMOOTHING_ENABLED
  s_slew.reset(s_appliedDuty, s_lastUpdateMs);
#endif
}

bool fanIsInFailsafe()
{
  return s_failsafe;
}

uint16_t fanGetRpm()
{
#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED
  return s_rpm;
#else
  return 0;
#endif
}
