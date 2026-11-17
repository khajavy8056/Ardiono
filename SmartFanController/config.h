#pragma once
/*
 * ============================================================================
 *   فایل تنظیمات مرکزی  —  پروژه «کنترل هوشمند دما و سرعت فن» (Arduino Uno)
 * ============================================================================
 *
 *   تمام تنظیمات پروژه از همین فایل خوانده می‌شود. برای تغییر رفتار برنامه
 *   کافی است مقادیر زیر را ویرایش کنید؛ نیازی به دستکاری منطق اصلی
 *   (فایل‌های .ino و .cpp) نیست.
 *
 *   نکته ۱: هر مقدار با #ifndef تعریف شده است تا بتوان آن را هنگام کامپایل
 *   با پرچم کامپایلر بازنویسی کرد (این کار در GitHub Actions برای تست
 *   چند پیکربندی مختلف انجام می‌شود). مثال:
 *       -DSENSOR_TYPE=3 -DDISPLAY_TYPE=0
 *
 *   نکته ۲: در انتهای این فایل، اعتبار مقادیر بررسی می‌شود و اگر مقداری
 *   نامعتبر باشد، کامپایل با پیام خطا متوقف می‌شود.
 * ============================================================================
 */

#include <Arduino.h>   // برای دسترسی به نام پایه‌های آنالوگ (A0..A5)

// ============================================================================
//   ۱) سنسور دما
// ============================================================================

// انتخاب نوع سنسور فعال (فقط یکی از سه حالت زیر)
#define SENSOR_DHT11  1   // سنسور DHT11
#define SENSOR_DHT22  2   // سنسور DHT22 (پیش‌فرض — دقت بهتر)
#define SENSOR_LM35   3   // سنسور آنالوگ LM35

#ifndef SENSOR_TYPE
#define SENSOR_TYPE SENSOR_DHT22
#endif

// پایه دیجیتال متصل به سنسور DHT (فقط وقتی سنسور DHT فعال است معنا دارد)
// مجاز: ۲ تا ۱۹ (پایه‌های دیجیتال و آنالوگ بردهای AVR)
#ifndef DHT_PIN
#define DHT_PIN 2
#endif

// پایه آنالوگ متصل به LM35 (فقط وقتی سنسور LM35 فعال است معنا دارد)
// مجاز: A0 تا A5
#ifndef LM35_PIN
#define LM35_PIN A0
#endif

// ولتاژ مرجع آنالوگ برد برای تبدیل خوانش LM35 (معمولاً ۵ ولت)
#ifndef LM35_VREF
#define LM35_VREF 5.0f
#endif

// اصلاح (calibration) دماي LM35 بر حسب درجه سلسیوس؛ برای جبران خطای سنسور
#ifndef LM35_OFFSET_C
#define LM35_OFFSET_C 0.0f
#endif

// فاصله زمانی نمونه‌برداری از سنسور (میلی‌ثانیه)
// نکته: برای DHT11 حداقل ۱۰۰۰ و برای DHT22 حداقل ۲۰۰۰ میلی‌ثانیه توصیه می‌شود.
#ifndef SENSOR_SAMPLE_INTERVAL_MS
#define SENSOR_SAMPLE_INTERVAL_MS 2000UL
#endif

// ---------------------------------------------------------------------------
// فیلتر نرم‌افزاری دما
// ---------------------------------------------------------------------------
// ۰ = غیرفعال، ۱ = فعال
// فیلتر میانگین متحرک نمایی (EMA) نوسانات سنسور را کاهش می‌دهد.
#ifndef TEMP_FILTER_ENABLED
#define TEMP_FILTER_ENABLED 1
#endif

// ضریب فیلتر EMA بین ۰ تا ۱ (مثلاً ۰٫۳۰)
// هرچه کوچک‌تر، فیلتر قوی‌تر و پاسخ کندتر؛ هرچه بزرگ‌تر، پاسخ سریع‌تر.
#ifndef TEMP_FILTER_ALPHA
#define TEMP_FILTER_ALPHA 0.30f
#endif

// حداکثر پرش مجاز دما بین دو نمونه متوالی (درجه سلسیوس).
// اگر اختلاف بیشتر از این مقدار شد، آن نمونه «مشکوک» در نظر گرفته و رد می‌شود.
// مقدار ۰ یعنی این قابلیت غیرفعال است.
#ifndef TEMP_MAX_JUMP_C
#define TEMP_MAX_JUMP_C 5.0f
#endif

// ---------------------------------------------------------------------------
// مدیریت خطای سنسور
// ---------------------------------------------------------------------------
// تعداد خطاهای پیاپی خواندن سنسور قبل از ورود به حالت ایمن (fail-safe)
#ifndef SENSOR_ERROR_THRESHOLD
#define SENSOR_ERROR_THRESHOLD 3
#endif

// سرعت فن در حالت ایمن (درصد). به‌طور کلی ۱۰۰ یعنی سرعت کامل.
#ifndef FAILSAFE_DUTY_PERCENT
#define FAILSAFE_DUTY_PERCENT 100
#endif

// ============================================================================
//   ۲) فن و مدار قدرت
// ============================================================================

// انتخاب نوع فن (فقط یکی از دو حالت زیر)
#define FAN_TYPE_DC_2WIRE  1   // فن DC دو سیمه (تغذیه + زمین) با درایور MOSFET
#define FAN_TYPE_DC_4WIRE  2   // فن DC چهار سیمه (تغذیه، زمین، PWM، تاخ)

#ifndef FAN_TYPE
#define FAN_TYPE FAN_TYPE_DC_2WIRE
#endif

// پایه خروجی PWM کنترل فن
// برای فن دو سیمه: یکی از پایه‌های PWM سخت‌افزاری (۳، ۵، ۶، ۹، ۱۰، ۱۱)
// برای فن چهار سیمه: حتماً ۹ یا ۱۰ (چون تغییر فرکانس تایمر ۱ روی این پایه‌هاست)
#ifndef FAN_PWM_PIN
#define FAN_PWM_PIN 9
#endif

// پایه ورودی سنسور سرعت (تاخ) فن چهار سیمه
// مجاز: ۲ یا ۳ (پایه‌های وقفه خارجی برد یونو)
#ifndef FAN4_TACH_PIN
#define FAN4_TACH_PIN 3
#endif

// ۰ = خواندن RPM غیرفعال، ۱ = فعال (فقط برای فن چهار سیمه)
#ifndef FAN4_TACH_ENABLED
#define FAN4_TACH_ENABLED 1
#endif

// فرکانس PWM فن چهار سیمه (هرتز). استاندارد فن‌های چهار سیمه ۲۵ کیلوهرتز است.
// محدوده مجاز: ۲۵۰ تا ۱۰۰۰۰۰ هرتز (با تایمر ۱ و prescaler=1)
#ifndef FAN4_PWM_FREQUENCY_HZ
#define FAN4_PWM_FREQUENCY_HZ 25000UL
#endif

// ---------------------------------------------------------------------------
// محدوده سرعت فن (درصد Duty Cycle)
// ---------------------------------------------------------------------------
// حداقل سرعت فن (۰ تا ۱۰۰)
#ifndef FAN_MIN_DUTY_PERCENT
#define FAN_MIN_DUTY_PERCENT 20
#endif

// حداکثر سرعت فن (۰ تا ۱۰۰)
#ifndef FAN_MAX_DUTY_PERCENT
#define FAN_MAX_DUTY_PERCENT 100
#endif

// اجازه توقف کامل فن در دمای پایین
// ۰ = فن حداقل سرعت پیش‌فرض را حفظ می‌کند (پیش‌فرض)
// ۱ = محدودیت حداقل سرعت غیرفعال می‌شود و فن در دمای کمتر از آستانه  ramp کامل می‌شود (توقف)
#ifndef FAN_ALLOW_STOP
#define FAN_ALLOW_STOP 0
#endif

// ---------------------------------------------------------------------------
// بازه دمایی کنترل (درجه سلسیوس)
// ---------------------------------------------------------------------------
// دمای شروع افزایش سرعت: در دماهای کمتر از این مقدار، فن در حداقل سرعت است.
#ifndef TEMP_RAMP_START_C
#define TEMP_RAMP_START_C 25.0f
#endif

// دمای رسیدن به حداکثر سرعت: در دماهای بیشتر از این مقدار، فن در حداکثر سرعت است.
#ifndef TEMP_RAMP_FULL_C
#define TEMP_RAMP_FULL_C 40.0f
#endif

// ---------------------------------------------------------------------------
// هموارسازی (smoothing) تغییر سرعت فن
// ---------------------------------------------------------------------------
// ۰ = غیرفعال، ۱ = فعال (پیش‌فرض)
// با فعال بودن، سرعت فن به‌تدریج و با نرخ مشخص به مقدار هدف می‌رسد.
#ifndef FAN_SMOOTHING_ENABLED
#define FAN_SMOOTHING_ENABLED 1
#endif

// حداکثر نرخ تغییر سرعت فن (درصد بر ثانیه)
// مثلاً ۴۰ یعنی فن حداکثر ۴۰ درصد در هر ثانیه سرعت را تغییر می‌دهد.
#ifndef FAN_SLEW_RATE_PERCENT_PER_SEC
#define FAN_SLEW_RATE_PERCENT_PER_SEC 40.0f
#endif

// فاصله زمانی به‌روزرسانی سرعت فن (میلی‌ثانیه)
#ifndef FAN_UPDATE_INTERVAL_MS
#define FAN_UPDATE_INTERVAL_MS 100UL
#endif

// ============================================================================
//   ۳) نمایشگر
// ============================================================================

// انتخاب نمایشگر (فقط یکی از سه حالت زیر)
#define DISPLAY_NONE        0   // نمایشگر کاملاً غیرفعال
#define DISPLAY_LCD16X2     1   // نمایشگر LCD 16x2 با ماژول I2C (پیش‌فرض)
#define DISPLAY_OLED128X64  2   // نمایشگر OLED 128x64 به‌صورت I2C

#ifndef DISPLAY_TYPE
#define DISPLAY_TYPE DISPLAY_LCD16X2
#endif

// آدرس I2C نمایشگر LCD (معمولاً 0x27 یا 0x3F)
#ifndef LCD_I2C_ADDRESS
#define LCD_I2C_ADDRESS 0x27
#endif

// آدرس I2C نمایشگر OLED (معمولاً 0x3C یا 0x3D)
#ifndef OLED_I2C_ADDRESS
#define OLED_I2C_ADDRESS 0x3C
#endif

// فاصله زمانی به‌روزرسانی نمایشگر (میلی‌ثانیه)
#ifndef DISPLAY_UPDATE_INTERVAL_MS
#define DISPLAY_UPDATE_INTERVAL_MS 500UL
#endif

// ============================================================================
//   ۴) گزارش سریال (Serial Monitor)
// ============================================================================

// ۰ = غیرفعال، ۱ = فعال (پیش‌فرض)
#ifndef SERIAL_ENABLED
#define SERIAL_ENABLED 1
#endif

// سرعت پورت سریال (بیت بر ثانیه)
#ifndef SERIAL_BAUD
#define SERIAL_BAUD 9600
#endif

// فاصله زمانی چاپ گزارش در Serial Monitor (میلی‌ثانیه)
#ifndef SERIAL_REPORT_INTERVAL_MS
#define SERIAL_REPORT_INTERVAL_MS 1000UL
#endif

// ============================================================================
//   ۵) اعتبارسنجی مقادیر (به‌صورت خودکار هنگام کامپایل)
// ============================================================================
//   اگر مقداری نامعتبر باشد، کامپایل با پیام خطا متوقف می‌شود تا کاربر
//   تصحیح کند. بخش عمده‌ای از خطاهای رایج تنظیمات را این‌جا می‌گیریم.

// --- نوع سنسور ---
#if (SENSOR_TYPE != SENSOR_DHT11) && (SENSOR_TYPE != SENSOR_DHT22) && (SENSOR_TYPE != SENSOR_LM35)
#error "SENSOR_TYPE must be SENSOR_DHT11, SENSOR_DHT22 or SENSOR_LM35"
#endif

// --- پایه سنسور DHT ---
#if (SENSOR_TYPE == SENSOR_DHT11) || (SENSOR_TYPE == SENSOR_DHT22)
  #if (DHT_PIN < 2) || (DHT_PIN > 19)
  #error "DHT_PIN must be between 2 and 19"
  #endif
  #if (SENSOR_SAMPLE_INTERVAL_MS < 1000UL)
  #error "SENSOR_SAMPLE_INTERVAL_MS must be at least 1000 ms for DHT sensors"
  #endif
#endif

// --- پایه سنسور LM35 ---
#if (SENSOR_TYPE == SENSOR_LM35)
  #if !((LM35_PIN == A0) || (LM35_PIN == A1) || (LM35_PIN == A2) || \
        (LM35_PIN == A3) || (LM35_PIN == A4) || (LM35_PIN == A5))
  #error "LM35_PIN must be one of the analog pins A0..A5"
  #endif
#endif

// --- ولتاژ مرجع و فیلتر ---
static_assert(LM35_VREF > 0.0f, "LM35_VREF must be greater than zero");
static_assert(TEMP_FILTER_ALPHA > 0.0f && TEMP_FILTER_ALPHA <= 1.0f,
              "TEMP_FILTER_ALPHA must be in (0, 1]");
static_assert(TEMP_MAX_JUMP_C >= 0.0f, "TEMP_MAX_JUMP_C must be >= 0");

// --- مدیریت خطا ---
#if (SENSOR_ERROR_THRESHOLD < 1)
#error "SENSOR_ERROR_THRESHOLD must be at least 1"
#endif
#if (FAILSAFE_DUTY_PERCENT < 0) || (FAILSAFE_DUTY_PERCENT > 100)
#error "FAILSAFE_DUTY_PERCENT must be between 0 and 100"
#endif

// --- نوع فن ---
#if (FAN_TYPE != FAN_TYPE_DC_2WIRE) && (FAN_TYPE != FAN_TYPE_DC_4WIRE)
#error "FAN_TYPE must be FAN_TYPE_DC_2WIRE or FAN_TYPE_DC_4WIRE"
#endif

// --- پایه PWM فن ---
#if (FAN_TYPE == FAN_TYPE_DC_2WIRE)
  #if !((FAN_PWM_PIN == 3) || (FAN_PWM_PIN == 5) || (FAN_PWM_PIN == 6) || \
        (FAN_PWM_PIN == 9) || (FAN_PWM_PIN == 10) || (FAN_PWM_PIN == 11))
  #error "FAN_PWM_PIN must be a hardware PWM pin (3, 5, 6, 9, 10, 11)"
  #endif
#else
  #if (FAN_PWM_PIN != 9) && (FAN_PWM_PIN != 10)
  #error "For a 4-wire fan, FAN_PWM_PIN must be 9 or 10 (Timer1)"
  #endif
#endif

// --- تاخ فن چهار سیمه ---
#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED
  #if (FAN4_TACH_PIN != 2) && (FAN4_TACH_PIN != 3)
  #error "FAN4_TACH_PIN must be 2 or 3 (external interrupt pins)"
  #endif
#endif

// --- فرکانس PWM فن چهار سیمه ---
#if (FAN_TYPE == FAN_TYPE_DC_4WIRE)
  #if (FAN4_PWM_FREQUENCY_HZ < 250UL) || (FAN4_PWM_FREQUENCY_HZ > 100000UL)
  #error "FAN4_PWM_FREQUENCY_HZ must be between 250 and 100000"
  #endif
#endif

// --- تداخل پایه‌ها ---
#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED
  #if (FAN_PWM_PIN == FAN4_TACH_PIN)
  #error "FAN_PWM_PIN and FAN4_TACH_PIN must be different"
  #endif
#endif
#if (SENSOR_TYPE != SENSOR_LM35) && (DHT_PIN == FAN_PWM_PIN)
#error "DHT_PIN and FAN_PWM_PIN must be different"
#endif
#if (SENSOR_TYPE != SENSOR_LM35) && (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED && (DHT_PIN == FAN4_TACH_PIN)
#error "DHT_PIN and FAN4_TACH_PIN must be different"
#endif

// --- محدوده سرعت ---
#if (FAN_MIN_DUTY_PERCENT < 0) || (FAN_MIN_DUTY_PERCENT > 100)
#error "FAN_MIN_DUTY_PERCENT must be between 0 and 100"
#endif
#if (FAN_MAX_DUTY_PERCENT < 0) || (FAN_MAX_DUTY_PERCENT > 100)
#error "FAN_MAX_DUTY_PERCENT must be between 0 and 100"
#endif
#if (FAN_MIN_DUTY_PERCENT > FAN_MAX_DUTY_PERCENT)
#error "FAN_MIN_DUTY_PERCENT must not be greater than FAN_MAX_DUTY_PERCENT"
#endif

// --- بازه دمایی ---
static_assert(TEMP_RAMP_FULL_C > TEMP_RAMP_START_C,
              "TEMP_RAMP_FULL_C must be greater than TEMP_RAMP_START_C");

// --- هموارسازی ---
static_assert(FAN_SLEW_RATE_PERCENT_PER_SEC > 0.0f,
              "FAN_SLEW_RATE_PERCENT_PER_SEC must be greater than zero");
#if (FAN_UPDATE_INTERVAL_MS < 20UL)
#error "FAN_UPDATE_INTERVAL_MS must be at least 20 ms"
#endif

// --- نمایشگر ---
#if (DISPLAY_TYPE != DISPLAY_NONE) && (DISPLAY_TYPE != DISPLAY_LCD16X2) && (DISPLAY_TYPE != DISPLAY_OLED128X64)
#error "DISPLAY_TYPE must be DISPLAY_NONE, DISPLAY_LCD16X2 or DISPLAY_OLED128X64"
#endif
#if (DISPLAY_UPDATE_INTERVAL_MS < 100UL)
#error "DISPLAY_UPDATE_INTERVAL_MS must be at least 100 ms"
#endif

// --- سریال ---
#if (SERIAL_REPORT_INTERVAL_MS < 100UL)
#error "SERIAL_REPORT_INTERVAL_MS must be at least 100 ms"
#endif
