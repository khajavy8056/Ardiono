/*
 * ============================================================================
 *   پیاده‌سازی ماژول سنسور دما
 * ============================================================================
 *   با توجه به SENSOR_TYPE یکی از سه بخش زیر کامپایل می‌شود:
 *     • DHT11 / DHT22 : استفاده از کتابخانه Adafruit DHT
 *     • LM35          : تبدیل خوانش آنالوگ به دما با رابط کاربری یکدست
 * ============================================================================
 */

#include "sensors.h"
#include <math.h>

#if (SENSOR_TYPE == SENSOR_DHT11) || (SENSOR_TYPE == SENSOR_DHT22)

#include <DHT.h>

// شیء سراسری سنسور DHT؛ نوع سنسور در زمان کامپایل تعیین می‌شود.
static DHT g_dht(DHT_PIN, (SENSOR_TYPE == SENSOR_DHT11) ? DHT11 : DHT22);

#endif

// ----------------------------------------------------------------------------
const char* sensorGetName()
{
#if (SENSOR_TYPE == SENSOR_DHT11)
  return "DHT11";
#elif (SENSOR_TYPE == SENSOR_DHT22)
  return "DHT22";
#elif (SENSOR_TYPE == SENSOR_LM35)
  return "LM35";
#else
  return "UNKNOWN";
#endif
}

// ----------------------------------------------------------------------------
void sensorBegin()
{
#if (SENSOR_TYPE == SENSOR_DHT11) || (SENSOR_TYPE == SENSOR_DHT22)
  g_dht.begin();
#elif (SENSOR_TYPE == SENSOR_LM35)
  pinMode(LM35_PIN, INPUT);
  analogReference(DEFAULT);   // مرجع آنالوگ پیش‌فرض برد (۵ ولت در یونو)
#endif
}

// ----------------------------------------------------------------------------
bool sensorRead(float &temperatureC, float &humidity)
{
#if (SENSOR_TYPE == SENSOR_DHT11) || (SENSOR_TYPE == SENSOR_DHT22)

  // نکته مهم: اگر هر دو مقدار را جداگانه بخوانیم، کتابخانه DHT در خواندن
  // دوم (اگر کمتر از MIN_INTERVAL گذشته باشد) همان نتیجه قبلی را برمی‌گرداند
  // و سنسور را دوباره نمی‌خواند؛ بنابراین هر دو مقدار از یک خواندن واحد
  // می‌آیند و فاصله زمانی نیز نقض نمی‌شود.
  const float t = g_dht.readTemperature();
  const float h = g_dht.readHumidity();

  // در صورت بروز خطا کتابخانه معمولاً NAN برمی‌گرداند.
  if (isnan(t) || isnan(h)) {
    return false;
  }

  temperatureC = t;
  humidity = h;
  return true;

#elif (SENSOR_TYPE == SENSOR_LM35)

  // LM35 هر ۱۰ میلی‌ولت برابر ۱ درجه سلسیوس خروجی می‌دهد.
  const int adc = analogRead(LM35_PIN);
  if (adc < 0 || adc > 1023) {
    return false;
  }

  const float voltage = ((float)adc) * (LM35_VREF / 1023.0f);
  temperatureC = (voltage * 100.0f) + LM35_OFFSET_C;
  humidity = 0.0f;   // LM35 رطوبت ندارد
  return true;

#else
  // حالت غیرممکن است (config.h آن را می‌گیرد)، ولی برای اطمینان:
  temperatureC = 0.0f;
  humidity = 0.0f;
  return false;
#endif
}
