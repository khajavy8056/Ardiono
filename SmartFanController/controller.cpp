/*
 * ============================================================================
 *   پیاده‌سازی ماژول کنترل‌کننده
 * ============================================================================
 *   الگوریتم کنترل:
 *     ۱. هر SENSOR_SAMPLE_INTERVAL_MS یک بار دما خوانده می‌شود.
 *     ۲. نمونه‌های مشکوک (پرش زیاد) رد و بقیه با فیلتر EMA هموار می‌شوند.
 *     ۳. دما به‌صورت پیوسته (خطی) به درصد سرعت فن نگاشت می‌شود:
 *           دما ≤ TEMP_RAMP_START_C  → حداقل سرعت (یا توقف)
 *           دما ≥ TEMP_RAMP_FULL_C   → حداکثر سرعت
 *           بین این دو                 → درصد بین حداقل و حداکثر
 *     ۴. تغییر سرعت فن با SlewRateLimiter به‌تدریج انجام می‌شود.
 *     ۵. در صورت SENSOR_ERROR_THRESHOLD خطای پیاپی، فن به حالت ایمن
 *        (سرعت کامل) می‌رود و پس از برگشت داده معتبر، کنترل از سر گرفته
 *        می‌شود.
 * ============================================================================
 */

#include "controller.h"
#include <math.h>

#include "sensors.h"
#include "fan.h"
#include "display.h"
#include "filters.h"

// ----------------------------------------------------------------------------
// وضعیت داخلی کنترل‌کننده
// ----------------------------------------------------------------------------
static LowPassFilter s_tempFilter(TEMP_FILTER_ALPHA);

static float    s_lastTempC        = 0.0f;   // آخرین دماي خام معتبر
static float    s_lastHumidity     = 0.0f;   // آخرین رطوبت معتبر
static float    s_filteredTempC    = 0.0f;   // دماي فیلترشده
static bool     s_hasFilteredValue = false;  // آیا دماي فیلترشده موجود است؟
static bool     s_sensorOk         = false;  // سنسور در حال ارائه داده معتبر
static uint16_t s_consecutiveErrors = 0;     // تعداد خطاهای پیاپی
static bool     s_failsafe         = false;  // وضعیت حالت ایمن

static uint32_t s_lastSampleMs  = 0UL;
static uint32_t s_lastFanMs     = 0UL;
static uint32_t s_lastDisplayMs = 0UL;
static uint32_t s_lastSerialMs  = 0UL;

// ----------------------------------------------------------------------------
// سرعت اولیه فن قبل از اولین خواندن معتبر سنسور
// ----------------------------------------------------------------------------
static float fanIdleDuty()
{
#if FAN_ALLOW_STOP
  return 0.0f;
#else
  return (float)FAN_MIN_DUTY_PERCENT;
#endif
}

// ----------------------------------------------------------------------------
// نگاشت پیوسته دما به درصد سرعت فن
// ----------------------------------------------------------------------------
static float mapTemperatureToDuty(float temperatureC)
{
  const float baseDuty = FAN_ALLOW_STOP ? 0.0f : (float)FAN_MIN_DUTY_PERCENT;

  if (temperatureC <= TEMP_RAMP_START_C) {
    return baseDuty;
  }
  if (temperatureC >= TEMP_RAMP_FULL_C) {
    return (float)FAN_MAX_DUTY_PERCENT;
  }

  // در بازه میانی: درصد به‌صورت خطی بین حداقل و حداکثر
  const float k = (temperatureC - TEMP_RAMP_START_C) /
                  (TEMP_RAMP_FULL_C - TEMP_RAMP_START_C);
  return baseDuty + k * ((float)FAN_MAX_DUTY_PERCENT - baseDuty);
}

// ----------------------------------------------------------------------------
// گزارش‌های Serial
// ----------------------------------------------------------------------------
static void reportSerial()
{
#if SERIAL_ENABLED
  Serial.print(F("T="));
  Serial.print(s_lastTempC, 1);
  Serial.print(F("C"));

#if (SENSOR_TYPE == SENSOR_DHT11) || (SENSOR_TYPE == SENSOR_DHT22)
  Serial.print(F(" H="));
  Serial.print(s_lastHumidity, 0);
  Serial.print(F("%"));
#endif

  Serial.print(F(" Fan="));
  Serial.print(fanGetCurrentDutyPercent(), 0);
  Serial.print(F("%"));

#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED
  Serial.print(F(" RPM="));
  Serial.print(fanGetRpm());
#endif

  Serial.print(F(" State="));
  if (s_failsafe) {
    Serial.println(F("FAILSAFE"));
  } else if (s_sensorOk) {
    Serial.println(F("AUTO"));
  } else {
    Serial.println(F("INIT"));
  }
#endif
}

static void reportSensorError()
{
#if SERIAL_ENABLED
  Serial.print(F("ERROR: sensor read failed ("));
  Serial.print(s_consecutiveErrors);
  Serial.print(F("/"));
  Serial.print(SENSOR_ERROR_THRESHOLD);
  Serial.println(F(") -> FAILSAFE (fan 100%)"));
#endif
}

static void reportRecovered()
{
#if SERIAL_ENABLED
  Serial.println(F("Sensor recovered -> AUTO control resumed"));
#endif
}

// ----------------------------------------------------------------------------
void controllerBegin()
{
#if SERIAL_ENABLED
  Serial.begin(SERIAL_BAUD);
  delay(100);   // کمی صبر برای آماده شدن پورت سریال (فقط در startup)

  Serial.println();
  Serial.println(F("=== Smart Fan Controller ==="));
  Serial.print(F("Sensor : ")); Serial.println(sensorGetName());
  Serial.print(F("Fan    : "));
#if (FAN_TYPE == FAN_TYPE_DC_2WIRE)
  Serial.println(F("2-wire DC (MOSFET)"));
#else
  Serial.println(F("4-wire PWM"));
#endif
  Serial.print(F("Display: "));
#if (DISPLAY_TYPE == DISPLAY_NONE)
  Serial.println(F("none"));
#elif (DISPLAY_TYPE == DISPLAY_LCD16X2)
  Serial.println(F("LCD 16x2 I2C"));
#else
  Serial.println(F("OLED 128x64 I2C"));
#endif
  Serial.print(F("Ramp   : "));
  Serial.print(TEMP_RAMP_START_C, 1);
  Serial.print(F("C -> "));
  Serial.print(TEMP_RAMP_FULL_C, 1);
  Serial.println(F("C"));
  Serial.print(F("Speed  : "));
  Serial.print(FAN_MIN_DUTY_PERCENT);
  Serial.print(F("% .. "));
  Serial.print(FAN_MAX_DUTY_PERCENT);
  Serial.println(F("%"));
  Serial.println(F("============================="));
#endif

  sensorBegin();
  fanBegin();
  displayBegin();

  // در ابتدای کار فن در حداقل سرعت (یا توقف) قرار می‌گیرد تا روشن شدن
  // ناگهانی با سرعت بالا نداشته باشیم.
  fanSetTargetDutyPercent(fanIdleDuty());
  fanUpdate(0UL);
}

// ----------------------------------------------------------------------------
void controllerTick(uint32_t nowMs)
{
  // ------------------------------------------------------------------
  // ۱) نمونه‌برداری از سنسور در بازه زمانی تنظیم‌شده
  // ------------------------------------------------------------------
  if (nowMs - s_lastSampleMs >= SENSOR_SAMPLE_INTERVAL_MS) {
    s_lastSampleMs = nowMs;

    float temperature = 0.0f;
    float humidity = 0.0f;

    if (sensorRead(temperature, humidity)) {
      // خواندن موفق
      s_consecutiveErrors = 0;

      if (!s_sensorOk) {
        if (s_failsafe) {
          s_failsafe = false;
          fanExitFailsafe();
          reportRecovered();
        }
        s_sensorOk = true;
      }

      // رد نوسان شدید (spike) در صورت فعال بودن این قابلیت
      const bool spike = (TEMP_MAX_JUMP_C > 0.0f) &&
                         s_hasFilteredValue &&
                         (fabsf(temperature - s_lastTempC) > TEMP_MAX_JUMP_C);

      if (!spike) {
        s_lastTempC = temperature;
        s_lastHumidity = humidity;
        s_filteredTempC = TEMP_FILTER_ENABLED ? s_tempFilter.update(temperature)
                                              : temperature;
        s_hasFilteredValue = true;
      }
    } else {
      // خواندن ناموفق
      s_consecutiveErrors++;

      if (!s_failsafe && (s_consecutiveErrors >= SENSOR_ERROR_THRESHOLD)) {
        s_failsafe = true;
        s_sensorOk = false;
        fanEnterFailsafe();   // فن بی‌درنگ به سرعت کامل می‌رود
        reportSensorError();
      }
    }
  }

  // ------------------------------------------------------------------
  // ۲) محاسبه و اعمال سرعت فن
  // ------------------------------------------------------------------
  if (nowMs - s_lastFanMs >= FAN_UPDATE_INTERVAL_MS) {
    s_lastFanMs = nowMs;

    if (s_sensorOk) {
      fanSetTargetDutyPercent(mapTemperatureToDuty(s_filteredTempC));
    }
    fanUpdate(nowMs);
  }

  // ------------------------------------------------------------------
  // ۳) به‌روزرسانی نمایشگر
  // ------------------------------------------------------------------
  if (nowMs - s_lastDisplayMs >= DISPLAY_UPDATE_INTERVAL_MS) {
    s_lastDisplayMs = nowMs;

    DisplayData data;
    data.temperatureC = s_lastTempC;
    data.humidity = s_lastHumidity;
    data.dutyPercent = fanGetCurrentDutyPercent();
    data.rpm = fanGetRpm();
    data.sensorOk = s_sensorOk;
    data.failsafe = s_failsafe;

    displayUpdate(data);
  }

  // ------------------------------------------------------------------
  // ۴) گزارش دوره‌ای در Serial Monitor
  // ------------------------------------------------------------------
  if (nowMs - s_lastSerialMs >= SERIAL_REPORT_INTERVAL_MS) {
    s_lastSerialMs = nowMs;
    reportSerial();
  }
}
