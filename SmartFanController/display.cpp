/*
 * ============================================================================
 *   پیاده‌سازی ماژول نمایشگر
 * ============================================================================
 *   با توجه به DISPLAY_TYPE یکی از سه بخش زیر کامپایل می‌شود:
 *     • DISPLAY_NONE        : بدون نمایشگر (همه توابع خالی)
 *     • DISPLAY_LCD16X2     : LCD 16x2 با ماژول I2C (LiquidCrystal_I2C)
 *     • DISPLAY_OLED128X64  : OLED 128x64 با I2C (Adafruit SSD1306)
 * ============================================================================
 */

#include "display.h"

#if (DISPLAY_TYPE == DISPLAY_LCD16X2)

#include <LiquidCrystal_I2C.h>

// سازنده: آدرس I2C، تعداد ستون‌ها، تعداد ردیف‌ها
static LiquidCrystal_I2C g_lcd(LCD_I2C_ADDRESS, 16, 2);

#elif (DISPLAY_TYPE == DISPLAY_OLED128X64)

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// سازنده: عرض، ارتفاع، شیء I2C، پایه reset (−۱ یعنی بدون reset سخت‌افزاری)
static Adafruit_SSD1306 g_oled(128, 64, &Wire, -1);

#endif

// ----------------------------------------------------------------------------
void displayBegin()
{
#if (DISPLAY_TYPE == DISPLAY_LCD16X2)

  g_lcd.init();
  g_lcd.backlight();
  g_lcd.clear();
  g_lcd.setCursor(0, 0);
  g_lcd.print("Smart Fan Ready");

#elif (DISPLAY_TYPE == DISPLAY_OLED128X64)

  if (!g_oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    // اگر نمایشگر پیدا نشد، کار برنامه ادامه می‌یابد (فقط گزارش سریال داریم)
  }
  g_oled.clearDisplay();
  g_oled.setTextSize(1);
  g_oled.setTextColor(SSD1306_WHITE);
  g_oled.setCursor(0, 0);
  g_oled.println("Smart Fan Ready");
  g_oled.display();

#endif
}

// ----------------------------------------------------------------------------
void displayUpdate(const DisplayData &d)
{
#if (DISPLAY_TYPE == DISPLAY_LCD16X2)

  g_lcd.clear();

  if (!d.sensorOk) {
    g_lcd.setCursor(0, 0);
    g_lcd.print("SENSOR ERROR");
    g_lcd.setCursor(0, 1);
    g_lcd.print("FAILSAFE ");
    g_lcd.print(d.dutyPercent, 0);
    g_lcd.print("%");
    return;
  }

  // ردیف اول: دما (+ رطوبت برای سنسورهای DHT)
  g_lcd.setCursor(0, 0);
  g_lcd.print("T:");
  g_lcd.print(d.temperatureC, 1);
  g_lcd.print("C");

#if (SENSOR_TYPE == SENSOR_DHT11) || (SENSOR_TYPE == SENSOR_DHT22)
  g_lcd.print(" H:");
  g_lcd.print(d.humidity, 0);
  g_lcd.print("%");
#endif

  // ردیف دوم: سرعت فن (+ RPM برای فن چهار سیمه)
  g_lcd.setCursor(0, 1);
  g_lcd.print("Fan:");
  g_lcd.print(d.dutyPercent, 0);
  g_lcd.print("%");

#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED
  g_lcd.print(" ");
  g_lcd.print(d.rpm);
  g_lcd.print("RPM");
#endif

#elif (DISPLAY_TYPE == DISPLAY_OLED128X64)

  g_oled.clearDisplay();
  g_oled.setTextSize(1);
  g_oled.setTextColor(SSD1306_WHITE);

  if (!d.sensorOk) {
    g_oled.setCursor(0, 0);
    g_oled.setTextSize(2);
    g_oled.println("SENSOR");
    g_oled.println("ERROR");
    g_oled.setTextSize(1);
    g_oled.print("Fan: ");
    g_oled.print(d.dutyPercent, 0);
    g_oled.println("% (FAILSAFE)");
    g_oled.display();
    return;
  }

  // دما
  g_oled.setCursor(0, 0);
  g_oled.setTextSize(2);
  g_oled.print(d.temperatureC, 1);
  g_oled.println(" C");

  // رطوبت (فقط DHT)
#if (SENSOR_TYPE == SENSOR_DHT11) || (SENSOR_TYPE == SENSOR_DHT22)
  g_oled.setTextSize(1);
  g_oled.setCursor(0, 18);
  g_oled.print("Humidity: ");
  g_oled.print(d.humidity, 0);
  g_oled.println("%");
#endif

  // سرعت فن
  g_oled.setTextSize(1);
  g_oled.setCursor(0, 30);
  g_oled.print("Fan: ");
  g_oled.print(d.dutyPercent, 0);
  g_oled.println("%");

#if (FAN_TYPE == FAN_TYPE_DC_4WIRE) && FAN4_TACH_ENABLED
  g_oled.setCursor(0, 42);
  g_oled.print("RPM: ");
  g_oled.println(d.rpm);
#endif

  g_oled.display();

#endif
}
