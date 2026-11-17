#pragma once
/*
 * ============================================================================
 *   ماژول فیلترها و هموارسازی
 * ============================================================================
 *   دو کلاس کوچک و بدون تخصیص حافظه پویا:
 *     • LowPassFilter   : فیلتر میانگین متحرک نمایی (EMA) برای دما
 *     • SlewRateLimiter : محدودکننده نرخ تغییر برای هموارسازی سرعت فن
 * ============================================================================
 */

#include <Arduino.h>

// ----------------------------------------------------------------------------
// فیلتر پایین‌گذر نمایی (EMA)
//   مقدار جدید = مقدار قبلی + آلفا × (نمونه جدید − مقدار قبلی)
// ----------------------------------------------------------------------------
class LowPassFilter
{
public:
  explicit LowPassFilter(float alpha) : m_alpha(alpha), m_value(0.0f), m_ready(false) {}

  // ثبت یک نمونه جدید و دریافت مقدار فیلترشده
  float update(float sample)
  {
    if (!m_ready) {
      m_value = sample;
      m_ready = true;
    } else {
      m_value = m_value + m_alpha * (sample - m_value);
    }
    return m_value;
  }

  // پاک‌سازی حالت فیلتر (مثلاً پس از خطای سنسور)
  void reset()
  {
    m_ready = false;
    m_value = 0.0f;
  }

  float getValue() const { return m_value; }
  bool  isReady() const { return m_ready; }

private:
  float m_alpha;
  float m_value;
  bool  m_ready;
};

// ----------------------------------------------------------------------------
// محدودکننده نرخ تغییر (Slew Rate Limiter)
//   مقدار خروجی هر بار حداکثر به میزان (نرخ × زمان گذشته) به هدف نزدیک می‌شود.
// ----------------------------------------------------------------------------
class SlewRateLimiter
{
public:
  explicit SlewRateLimiter(float maxStepPerSec)
    : m_maxStepPerSec(maxStepPerSec), m_value(0.0f), m_lastMs(0UL), m_ready(false) {}

  // target : مقدار هدف (درصد)
  // nowMs  : زمان فعلی بر حسب میلی‌ثانیه (millis)
  float update(float target, uint32_t nowMs)
  {
    if (!m_ready) {
      m_value = target;
      m_lastMs = nowMs;
      m_ready = true;
      return m_value;
    }

    const uint32_t dtMs = nowMs - m_lastMs;
    m_lastMs = nowMs;

    if (dtMs == 0UL) {
      return m_value;
    }

    const float dtSec = ((float)dtMs) / 1000.0f;
    float maxStep = m_maxStepPerSec * dtSec;

    // اگر هدف نزدیک‌تر از یک گام باشد، مستقیم به هدف می‌رویم.
    const float diff = target - m_value;
    if (diff > 0.0f) {
      m_value += (diff < maxStep) ? diff : maxStep;
    } else if (diff < 0.0f) {
      const float absDiff = -diff;
      const float step = (absDiff < maxStep) ? absDiff : maxStep;
      m_value -= step;
    }

    return m_value;
  }

  // تنظیم مجدد مقدار خروجی (مثلاً پس از خروج از حالت ایمن) بدون جهش سرعت
  void reset(float value, uint32_t nowMs)
  {
    m_value = value;
    m_lastMs = nowMs;
    m_ready = true;
  }

  float getValue() const { return m_value; }

private:
  float   m_maxStepPerSec;
  float   m_value;
  uint32_t m_lastMs;
  bool    m_ready;
};
