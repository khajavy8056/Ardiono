#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
============================================================================
  اسکریپت تولید تصاویر شماتیک (نقشه� اتصالات) پروژهٔ کنترل هوشمند دما و فن
============================================================================
  این اسکریپت با کمک کتابخانهٔ matplotlib سه تصویر شماتیک تولید می‌کند:

    1) schematic-default.png  — پیکربندی پیش‌فرض: DHT22 + LCD I2C + فن دو سیمه
    2) schematic-lm35.png     — سنسور LM35 + LCD I2C + فن دو سیمه
    3) schematic-4wire.png    — DHT22 + OLED I2C + فن چهار سیمه (PWM + TACH)

  پیش‌نیازها:
      pip install matplotlib arabic-reshaper python-bidi
  فونت فارسی Vazirmatn در پوشهٔ fonts (کنار این اسکریپت) قرار می‌گیرد.
============================================================================
"""

import os
import arabic_reshaper
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Circle, Polygon, Rectangle
from matplotlib.font_manager import FontProperties
from bidi.algorithm import get_display

HERE = os.path.dirname(os.path.abspath(__file__))
FONT_REG = os.path.join(HERE, "fonts", "Vazirmatn-Regular.ttf")
FONT_BOLD = os.path.join(HERE, "fonts", "Vazirmatn-Bold.ttf")

font_r = FontProperties(fname=FONT_REG)
font_b = FontProperties(fname=FONT_BOLD)

# رنگ‌های استاندارد سیم‌ها
C_POWER = "#d62728"   # قرمز — تغذیه
C_GND = "#222222"     # مشکی — زمین
C_DATA = "#2ca02c"    # سبز — داده� سنسور
C_PWM = "#9467bd"     # بنفش — PWM فن
C_SDA = "#1f77b4"     # آبی — SDA
C_SCL = "#ff7f0e"     # نارنجی — SCL
C_TACH = "#17becf"    # فیروزه‌ای — تاخ
C_12V = "#d62728"     # قرمز — تغذیهٔ ۱۲ ولت


def fa(text):
    """تبدیل متن فارسی به حالت نمایشی برای رسم در matplotlib."""
    return get_display(arabic_reshaper.reshape(text))


class Sheet:
    """بوم رسم یک شماتیک."""

    def __init__(self, width, height, title, dpi=170):
        self.W = width
        self.H = height
        self.fig, self.ax = plt.subplots(figsize=(width / 100.0, height / 100.0), dpi=dpi)
        self.ax.set_xlim(0, width)
        self.ax.set_ylim(0, height)
        self.ax.set_aspect("equal")
        self.ax.axis("off")
        self.fig.patch.set_facecolor("white")
        # عنوان شماتیک
        self.ax.text(width / 2.0, height - 26, fa(title), ha="center", va="center",
                     fontproperties=font_b, fontsize=17, color="#111111")

    def box(self, x, y, w, h, title, lines=None, fc="#f4f7fb", ec="#33415c", size=12):
        rect = FancyBboxPatch((x, y), w, h,
                              boxstyle="round,pad=0.01,rounding_size=10",
                              linewidth=1.6, edgecolor=ec, facecolor=fc, zorder=2)
        self.ax.add_patch(rect)
        cy = y + h - 20
        self.ax.text(x + w / 2.0, cy, fa(title), ha="center", va="center",
                     fontproperties=font_b, fontsize=size, color=ec, zorder=3)
        if lines:
            for i, ln in enumerate(lines):
                self.ax.text(x + w / 2.0, cy - 22 - i * 20, fa(ln),
                             ha="center", va="center", fontproperties=font_r,
                             fontsize=10, color="#333333", zorder=3)

    def pin(self, x, y, color, label=None, side="right", dy=12):
        self.ax.add_patch(Circle((x, y), 6, color=color, zorder=5))
        if label:
            if side == "right":
                self.ax.text(x + 12, y, fa(label), ha="left", va="center",
                             fontproperties=font_b, fontsize=10, color="#111111", zorder=6)
            else:
                self.ax.text(x - 12, y, fa(label), ha="right", va="center",
                             fontproperties=font_b, fontsize=10, color="#111111", zorder=6)

    def wire(self, p1, p2, color, label=None, lw=3.4, label_dy=14, ls="--"):
        if ls == "--":
            self.ax.plot([p1[0], p2[0]], [p1[1], p2[1]], color=color, lw=lw,
                         solid_capstyle="round", linestyle=(0, (7, 5)), zorder=1,
                         alpha=0.9)
        else:
            self.ax.plot([p1[0], p2[0]], [p1[1], p2[1]], color=color, lw=lw,
                         solid_capstyle="round", zorder=1, alpha=0.9)
        self.ax.add_patch(Circle(p1, 4, color=color, zorder=4))
        self.ax.add_patch(Circle(p2, 4, color=color, zorder=4))
        if label:
            mx = (p1[0] + p2[0]) / 2.0
            my = (p1[1] + p2[1]) / 2.0 + label_dy
            self.ax.text(mx, my, fa(label), ha="center", va="bottom",
                         fontproperties=font_r, fontsize=9.5, color=color,
                         zorder=6,
                         bbox=dict(boxstyle="round,pad=0.15", fc="white",
                                   ec="none", alpha=0.85))

    def note(self, x, y, text, color="#555555", size=10):
        self.ax.text(x, y, fa(text), ha="center", va="center",
                     fontproperties=font_r, fontsize=size, color=color, zorder=6)

    def legend(self, x, y, items, title="راهنمای رنگ سیم‌ها"):
        self.ax.text(x, y, fa(title), ha="center", va="center",
                     fontproperties=font_b, fontsize=11, color="#111111", zorder=6)
        for i, (color, text) in enumerate(items):
            yy = y - 24 - i * 21
            self.ax.plot([x - 75, x - 45], [yy, yy], color=color, lw=3.4,
                         solid_capstyle="round", linestyle=(0, (7, 5)), zorder=6)
            self.ax.text(x - 20, yy, fa(text), ha="left", va="center",
                         fontproperties=font_r, fontsize=9.5, color="#333333", zorder=6)

    def save(self, name):
        path = os.path.join(HERE, name)
        self.fig.savefig(path, bbox_inches="tight", facecolor="white")
        plt.close(self.fig)
        print("saved:", path)


# ============================================================================
#  ۱) پیکربندی پیش‌فرض: DHT22 + LCD 16x2 I2C + فن دو سیمه با MOSFET
# ============================================================================
def draw_default():
    s = Sheet(1650, 1120, "شماتیک اتصالات — پیکربندی پیش‌فرض (DHT22 + LCD 16x2 + فن دو سیمه)")

    # --- آردوینو ---
    ax = 60
    ay = 330
    aw = 370
    ah = 480
    s.box(ax, ay, aw, ah, "Arduino Uno",
          ["5V   GND   D2   A4   A5   D9",
           "میکروکنترلر ATmega328P"])

    # پین‌های لبه� راست آردوینو
    pins = [
        ("5V", ax + aw, ay + ah - 70, C_POWER),
        ("GND", ax + aw, ay + ah - 130, C_GND),
        ("D2", ax + aw, ay + ah - 200, C_DATA),
        ("A4", ax + aw, ay + ah - 265, C_SDA),
        ("A5", ax + aw, ay + ah - 325, C_SCL),
        ("D9", ax + aw, ay + ah - 400, C_PWM),
    ]
    for name, px, py, col in pins:
        s.pin(px, py, col, name, side="right")

    # --- سنسور DHT22 ---
    dx, dy, dw, dh = 1230, 800, 350, 180
    s.box(dx, dy, dw, dh, "سنسور DHT22",
          ["VCC   GND   DATA", "دما و رطوبت"])
    dht = {"VCC": (dx, dy + dh - 45), "GND": (dx, dy + dh - 95), "DATA": (dx, dy + dh - 145)}
    for name, (px, py) in dht.items():
        s.pin(px, py, {"VCC": C_POWER, "GND": C_GND, "DATA": C_DATA}[name],
              name, side="left")

    # --- نمایشگر LCD I2C ---
    lx, ly, lw, lh = 1230, 540, 350, 200
    s.box(lx, ly, lw, lh, "نمایشگر LCD 16x2 با I2C",
          ["VCC   GND   SDA   SCL", "آدرس پیش‌فرض 0x27"])
    lcd = {"VCC": (lx, ly + lh - 45), "GND": (lx, ly + lh - 95),
           "SDA": (lx, ly + lh - 145), "SCL": (lx, ly + lh - 190)}
    for name, (px, py) in lcd.items():
        s.pin(px, py, {"VCC": C_POWER, "GND": C_GND, "SDA": C_SDA, "SCL": C_SCL}[name],
              name, side="left")

    # --- منبع تغذیه ۱۲ ولت ---
    px, py, pw, ph = 700, 90, 300, 110
    s.box(px, py, pw, ph, "منبع تغذیه ۱۲ ولت",
          ["+12V   GND", "جداگانه برای فن"])
    s.pin(px + pw - 20, py + ph - 35, C_12V, "+12V", side="right")
    s.pin(px + pw - 20, py + ph - 80, C_GND, "GND", side="right")

    # --- MOSFET و فن ---
    mx, my, mw, mh = 1130, 90, 500, 240
    s.box(mx, my, mw, mh, "مدار درایور فن (MOSFET)",
          ["IRLZ44N یا مشابه سطح‌منطق"])
    # جعبه MOSFET
    mos_x, mos_y, mos_w, mos_h = mx + 20, my + 40, 170, 150
    s.box(mos_x, mos_y, mos_w, mos_h, "MOSFET", [], fc="#eef3f9", ec="#555555", size=11)
    s.pin(mos_x, mos_y + mos_h - 35, C_PWM, "G", side="left")
    s.pin(mos_x + mos_w, mos_y + mos_h - 35, C_GND, "S", side="right")
    s.pin(mos_x + mos_w / 2.0, mos_y + mos_h, C_12V, "D", side="right", dy=14)

    # دیود فلایبک
    s.note(mx + 215, my + mh - 28, "دیود فلایبک 1N4007", color="#7a3e00", size=9)

    # فن (دایره)
    fan_c = (mx + mw - 90, my + 120)
    s.ax.add_patch(Circle(fan_c, 78, facecolor="#e8eef7", edgecolor="#33415c", lw=2, zorder=3))
    import math
    for ang in (0, 120, 240):
        a = math.radians(ang)
        s.ax.add_patch(Polygon([
            (fan_c[0], fan_c[1]),
            (fan_c[0] + 70 * math.cos(a - 0.35), fan_c[1] + 70 * math.sin(a - 0.35)),
            (fan_c[0] + 40 * math.cos(a), fan_c[1] + 40 * math.sin(a)),
            (fan_c[0] + 70 * math.cos(a + 0.35), fan_c[1] + 70 * math.sin(a + 0.35)),
        ], closed=True, facecolor="#9fb6d4", edgecolor="#33415c", lw=1.2, zorder=4))
    s.ax.add_patch(Circle(fan_c, 14, facecolor="#33415c", zorder=5))
    s.note(fan_c[0], fan_c[1] - 100, "فن DC دو سیمه")

    # --- سیم‌ها ---
    ard = {n: (px_, py_) for n, px_, py_, _ in pins}

    # 5V آردوینو → ریل تغذیه
    rail_x = 760
    s.wire((ard["5V"][0], ard["5V"][1]), (rail_x, ard["5V"][1]), C_POWER)
    s.wire((rail_x, ard["5V"][1]), (rail_x, dht["VCC"][1]), C_POWER)
    s.wire((rail_x, dht["VCC"][1]), dht["VCC"], C_POWER, "5V")
    s.wire((rail_x, lcd["VCC"][1]), lcd["VCC"], C_POWER, "5V")

    # GND آردوینو → ریل زمین
    gnd_x = 690
    s.wire((ard["GND"][0], ard["GND"][1]), (gnd_x, ard["GND"][1]), C_GND)
    s.wire((gnd_x, ard["GND"][1]), (gnd_x, dht["GND"][1]), C_GND)
    s.wire((gnd_x, dht["GND"][1]), dht["GND"], C_GND, "GND")
    s.wire((gnd_x, lcd["GND"][1]), lcd["GND"], C_GND, "GND")
    s.wire((gnd_x, mos_y + mos_h - 35), (mos_x + mos_w, mos_y + mos_h - 35), C_GND, "GND")

    # زمین مشترک با منبع ۱۲ ولت
    s.wire((px + pw - 20, py + ph - 80), (gnd_x, py + ph - 80), C_GND)
    s.wire((gnd_x, py + ph - 80), (gnd_x, ard["GND"][1]), C_GND, "زمین مشترک")

    # D2 → DATA
    s.wire((ard["D2"][0], ard["D2"][1]), dht["DATA"], C_DATA, "DATA (با پول‌آپ ۱۰ کیلوهم)")

    # A4/A5 → LCD
    s.wire((ard["A4"][0], ard["A4"][1]), lcd["SDA"], C_SDA, "SDA")
    s.wire((ard["A5"][0], ard["A5"][1]), lcd["SCL"], C_SCL, "SCL")

    # D9 → گیت MOSFET (با مقاومت)
    s.wire((ard["D9"][0], ard["D9"][1]), (mos_x, mos_y + mos_h - 35), C_PWM, "PWM (با مقاومت ۲۲۰ اهم)")

    # 12V+ → فن+
    s.wire((px + pw - 20, py + ph - 35), (fan_c[0], fan_c[1] + 78), C_12V, "+12V")

    # فن− → دراین MOSFET
    s.wire((fan_c[0] - 20, fan_c[1] - 72), (mos_x + mos_w / 2.0, mos_y + mos_h), C_12V, "Fan−")

    # راهنما
    s.legend(300, 250, [
        (C_POWER, "تغذیهٔ ۵ ولت"),
        (C_GND, "زمین (GND)"),
        (C_DATA, "دادهٔ سنسور DHT"),
        (C_PWM, "خروجی PWM فن"),
        (C_SDA, "SDA نمایشگر I2C"),
        (C_SCL, "SCL نمایشگر I2C"),
        (C_12V, "تغذیهٔ ۱۲ ولت فن"),
    ])

    s.note(825, 30, "تذکر ایمنی: فن را هرگز از پین ۵V آردوینو تغذیه نکنید — از منبع جدا و زمین مشترک استفاده کنید.",
            color="#a11212", size=10.5)
    s.save("schematic-default.png")


# ============================================================================
#  ۲) سنسور LM35 + LCD I2C + فن دو سیمه
# ============================================================================
def draw_lm35():
    s = Sheet(1650, 1060, "شماتیک اتصالات — سنسور LM35 + LCD 16x2 + فن دو سیمه")

    ax, ay, aw, ah = 60, 320, 370, 470
    s.box(ax, ay, aw, ah, "Arduino Uno",
          ["5V   GND   A0   A4   A5   D9",
           "میکروکنترلر ATmega328P"])

    pins = [
        ("5V", ax + aw, ay + ah - 70, C_POWER),
        ("GND", ax + aw, ay + ah - 130, C_GND),
        ("A0", ax + aw, ay + ah - 210, C_DATA),
        ("A4", ax + aw, ay + ah - 280, C_SDA),
        ("A5", ax + aw, ay + ah - 345, C_SCL),
        ("D9", ax + aw, ay + ah - 415, C_PWM),
    ]
    for name, px, py, col in pins:
        s.pin(px, py, col, name, side="right")

    # LM35
    sx, sy, sw, sh = 1230, 760, 350, 170
    s.box(sx, sy, sw, sh, "سنسور LM35",
          ["VCC   VOUT   GND", "هر ۱۰mV برابر ۱°C"])
    lm = {"VCC": (sx, sy + sh - 45), "VOUT": (sx, sy + sh - 95), "GND": (sx, sy + sh - 140)}
    for name, (px_, py_) in lm.items():
        col = C_POWER if name == "VCC" else (C_DATA if name == "VOUT" else C_GND)
        s.pin(px_, py_, col, name, side="left")

    # LCD
    lx, ly, lw, lh = 1230, 500, 350, 200
    s.box(lx, ly, lw, lh, "نمایشگر LCD 16x2 با I2C",
          ["VCC   GND   SDA   SCL", "آدرس پیش‌فرض 0x27"])
    lcd = {"VCC": (lx, ly + lh - 45), "GND": (lx, ly + lh - 95),
           "SDA": (lx, ly + lh - 145), "SCL": (lx, ly + lh - 190)}
    for name, (px_, py_) in lcd.items():
        s.pin(px_, py_, {"VCC": C_POWER, "GND": C_GND, "SDA": C_SDA, "SCL": C_SCL}[name],
              name, side="left")

    # منبع ۱۲ ولت
    px, py, pw, ph = 700, 90, 300, 110
    s.box(px, py, pw, ph, "منبع تغذیه ۱۲ ولت", ["+12V   GND", "جداگانه برای فن"])
    s.pin(px + pw - 20, py + ph - 35, C_12V, "+12V", side="right")
    s.pin(px + pw - 20, py + ph - 80, C_GND, "GND", side="right")

    # MOSFET + فن
    mx, my, mw, mh = 1130, 90, 500, 240
    s.box(mx, my, mw, mh, "مدار درایور فن (MOSFET)",
          ["IRLZ44N یا مشابه سطح‌منطق"])
    mos_x, mos_y, mos_w, mos_h = mx + 20, my + 40, 170, 150
    s.box(mos_x, mos_y, mos_w, mos_h, "MOSFET", [], fc="#eef3f9", ec="#555555", size=11)
    s.pin(mos_x, mos_y + mos_h - 35, C_PWM, "G", side="left")
    s.pin(mos_x + mos_w, mos_y + mos_h - 35, C_GND, "S", side="right")
    s.pin(mos_x + mos_w / 2.0, mos_y + mos_h, C_12V, "D", side="right", dy=14)
    s.note(mx + 215, my + mh - 28, "دیود فلایبک 1N4007", color="#7a3e00", size=9)

    fan_c = (mx + mw - 90, my + 120)
    s.ax.add_patch(Circle(fan_c, 78, facecolor="#e8eef7", edgecolor="#33415c", lw=2, zorder=3))
    import math
    for ang in (0, 120, 240):
        a = math.radians(ang)
        s.ax.add_patch(Polygon([
            (fan_c[0], fan_c[1]),
            (fan_c[0] + 70 * math.cos(a - 0.35), fan_c[1] + 70 * math.sin(a - 0.35)),
            (fan_c[0] + 40 * math.cos(a), fan_c[1] + 40 * math.sin(a)),
            (fan_c[0] + 70 * math.cos(a + 0.35), fan_c[1] + 70 * math.sin(a + 0.35)),
        ], closed=True, facecolor="#9fb6d4", edgecolor="#33415c", lw=1.2, zorder=4))
    s.ax.add_patch(Circle(fan_c, 14, facecolor="#33415c", zorder=5))
    s.note(fan_c[0], fan_c[1] - 100, "فن DC دو سیمه")

    ard = {n: (px_, py_) for n, px_, py_, _ in pins}

    rail_x = 760
    s.wire((ard["5V"][0], ard["5V"][1]), (rail_x, ard["5V"][1]), C_POWER)
    s.wire((rail_x, ard["5V"][1]), (rail_x, lm["VCC"][1]), C_POWER)
    s.wire((rail_x, lm["VCC"][1]), lm["VCC"], C_POWER, "5V")
    s.wire((rail_x, lcd["VCC"][1]), lcd["VCC"], C_POWER, "5V")

    gnd_x = 690
    s.wire((ard["GND"][0], ard["GND"][1]), (gnd_x, ard["GND"][1]), C_GND)
    s.wire((gnd_x, ard["GND"][1]), (gnd_x, lm["GND"][1]), C_GND)
    s.wire((gnd_x, lm["GND"][1]), lm["GND"], C_GND, "GND")
    s.wire((gnd_x, lcd["GND"][1]), lcd["GND"], C_GND, "GND")
    s.wire((gnd_x, mos_y + mos_h - 35), (mos_x + mos_w, mos_y + mos_h - 35), C_GND, "GND")
    s.wire((px + pw - 20, py + ph - 80), (gnd_x, py + ph - 80), C_GND)
    s.wire((gnd_x, py + ph - 80), (gnd_x, ard["GND"][1]), C_GND, "زمین مشترک")

    s.wire((ard["A0"][0], ard["A0"][1]), lm["VOUT"], C_DATA, "VOUT")
    s.wire((ard["A4"][0], ard["A4"][1]), lcd["SDA"], C_SDA, "SDA")
    s.wire((ard["A5"][0], ard["A5"][1]), lcd["SCL"], C_SCL, "SCL")
    s.wire((ard["D9"][0], ard["D9"][1]), (mos_x, mos_y + mos_h - 35), C_PWM, "PWM (با مقاومت ۲۲۰ اهم)")
    s.wire((px + pw - 20, py + ph - 35), (fan_c[0], fan_c[1] + 78), C_12V, "+12V")
    s.wire((fan_c[0] - 20, fan_c[1] - 72), (mos_x + mos_w / 2.0, mos_y + mos_h), C_12V, "Fan−")

    s.legend(300, 240, [
        (C_POWER, "تغذیهٔ ۵ ولت"),
        (C_GND, "زمین (GND)"),
        (C_DATA, "خروجی آنالوگ LM35"),
        (C_PWM, "خروجی PWM فن"),
        (C_SDA, "SDA نمایشگر I2C"),
        (C_SCL, "SCL نمایشگر I2C"),
        (C_12V, "تغذیهٔ ۱۲ ولت فن"),
    ])
    s.note(825, 30, "تذکر ایمنی: فن را هرگز از پین ۵V آردوینو تغذیه نکنید — از منبع جدا و زمین مشترک استفاده کنید.",
            color="#a11212", size=10.5)
    s.save("schematic-lm35.png")


# ============================================================================
#  ۳) DHT22 + OLED 128x64 I2C + فن چهار سیمه (PWM + TACH)
# ============================================================================
def draw_4wire():
    s = Sheet(1650, 1120, "شماتیک اتصالات — DHT22 + OLED 128x64 + فن چهار سیمه (PWM)")

    ax, ay, aw, ah = 60, 330, 370, 480
    s.box(ax, ay, aw, ah, "Arduino Uno",
          ["5V   GND   D2   A4   A5   D9   D3",
           "میکروکنترلر ATmega328P"])
    pins = [
        ("5V", ax + aw, ay + ah - 70, C_POWER),
        ("GND", ax + aw, ay + ah - 130, C_GND),
        ("D2", ax + aw, ay + ah - 200, C_DATA),
        ("A4", ax + aw, ay + ah - 265, C_SDA),
        ("A5", ax + aw, ay + ah - 325, C_SCL),
        ("D9", ax + aw, ay + ah - 400, C_PWM),
        ("D3", ax + aw, ay + ah - 455, C_TACH),
    ]
    for name, px, py, col in pins:
        s.pin(px, py, col, name, side="right")

    # DHT22
    dx, dy, dw, dh = 1230, 800, 350, 180
    s.box(dx, dy, dw, dh, "سنسور DHT22", ["VCC   GND   DATA", "دما و رطوبت"])
    dht = {"VCC": (dx, dy + dh - 45), "GND": (dx, dy + dh - 95), "DATA": (dx, dy + dh - 145)}
    for name, (px_, py_) in dht.items():
        s.pin(px_, py_, {"VCC": C_POWER, "GND": C_GND, "DATA": C_DATA}[name], name, side="left")

    # OLED
    ox, oy, ow, oh = 1230, 540, 350, 200
    s.box(ox, oy, ow, oh, "نمایشگر OLED 128x64 با I2C",
          ["VCC   GND   SDA   SCL", "آدرس پیش‌فرض 0x3C"])
    oled = {"VCC": (ox, oy + oh - 45), "GND": (ox, oy + oh - 95),
            "SDA": (ox, oy + oh - 145), "SCL": (ox, oy + oh - 190)}
    for name, (px_, py_) in oled.items():
        s.pin(px_, py_, {"VCC": C_POWER, "GND": C_GND, "SDA": C_SDA, "SCL": C_SCL}[name],
              name, side="left")

    # منبع ۱۲ ولت
    px, py, pw, ph = 700, 90, 300, 110
    s.box(px, py, pw, ph, "منبع تغذیه ۱۲ ولت", ["+12V   GND", "جداگانه برای فن"])
    s.pin(px + pw - 20, py + ph - 35, C_12V, "+12V", side="right")
    s.pin(px + pw - 20, py + ph - 80, C_GND, "GND", side="right")

    # فن چهار سیمه
    fx, fy, fw, fh = 1130, 90, 500, 240
    s.box(fx, fy, fw, fh, "فن DC چهار سیمه",
          ["قرمز +12V   سیاه GND", "آبی PWM   زرد TACH"])
    fan_c = (fx + 150, fy + 120)
    s.ax.add_patch(Circle(fan_c, 78, facecolor="#e8eef7", edgecolor="#33415c", lw=2, zorder=3))
    import math
    for ang in (0, 120, 240):
        a = math.radians(ang)
        s.ax.add_patch(Polygon([
            (fan_c[0], fan_c[1]),
            (fan_c[0] + 70 * math.cos(a - 0.35), fan_c[1] + 70 * math.sin(a - 0.35)),
            (fan_c[0] + 40 * math.cos(a), fan_c[1] + 40 * math.sin(a)),
            (fan_c[0] + 70 * math.cos(a + 0.35), fan_c[1] + 70 * math.sin(a + 0.35)),
        ], closed=True, facecolor="#9fb6d4", edgecolor="#33415c", lw=1.2, zorder=4))
    s.ax.add_patch(Circle(fan_c, 14, facecolor="#33415c", zorder=5))
    s.note(fan_c[0], fan_c[1] - 100, "فن چهار سیمه")

    # پین‌های فن
    fan_pins = {
        "قرمز (+12V)": (fx + fw - 200, fy + 200),
        "سیاه (GND)": (fx + fw - 200, fy + 160),
        "آبی (PWM)": (fx + fw - 200, fy + 120),
        "زرد (TACH)": (fx + fw - 200, fy + 80),
    }
    for name, (px_, py_) in fan_pins.items():
        s.pin(px_, py_, C_12V, name, side="right")

    # پول‌آپ تاخ
    s.ax.plot([fx + fw - 150, fx + fw - 150], [fy + 80, fy + 150], color="#777777", lw=1.4, zorder=3)
    s.note(fx + fw - 120, fy + 60, "پول‌آپ ۱۰ کیلوهم", color="#555555", size=9)

    ard = {n: (px_, py_) for n, px_, py_, _ in pins}

    rail_x = 760
    s.wire((ard["5V"][0], ard["5V"][1]), (rail_x, ard["5V"][1]), C_POWER)
    s.wire((rail_x, ard["5V"][1]), (rail_x, dht["VCC"][1]), C_POWER)
    s.wire((rail_x, dht["VCC"][1]), dht["VCC"], C_POWER, "5V")
    s.wire((rail_x, oled["VCC"][1]), oled["VCC"], C_POWER, "5V")

    gnd_x = 690
    s.wire((ard["GND"][0], ard["GND"][1]), (gnd_x, ard["GND"][1]), C_GND)
    s.wire((gnd_x, ard["GND"][1]), (gnd_x, dht["GND"][1]), C_GND)
    s.wire((gnd_x, dht["GND"][1]), dht["GND"], C_GND, "GND")
    s.wire((gnd_x, oled["GND"][1]), oled["GND"], C_GND, "GND")
    s.wire((px + pw - 20, py + ph - 80), (gnd_x, py + ph - 80), C_GND)
    s.wire((gnd_x, py + ph - 80), (gnd_x, ard["GND"][1]), C_GND, "زمین مشترک")

    s.wire((ard["D2"][0], ard["D2"][1]), dht["DATA"], C_DATA, "DATA (با پول‌آپ ۱۰ کیلوهم)")
    s.wire((ard["A4"][0], ard["A4"][1]), oled["SDA"], C_SDA, "SDA")
    s.wire((ard["A5"][0], ard["A5"][1]), oled["SCL"], C_SCL, "SCL")
    s.wire((ard["D9"][0], ard["D9"][1]), fan_pins["آبی (PWM)"], C_PWM, "PWM (~۲۵kHz)")
    s.wire((ard["D3"][0], ard["D3"][1]), fan_pins["زرد (TACH)"], C_TACH, "TACH (RPM)")
    s.wire((px + pw - 20, py + ph - 35), fan_pins["قرمز (+12V)"], C_12V, "+12V")
    s.wire((px + pw - 20, py + ph - 80), fan_pins["سیاه (GND)"], C_GND, "GND")

    s.legend(300, 250, [
        (C_POWER, "تغذیهٔ ۵ ولت"),
        (C_GND, "زمین (GND)"),
        (C_DATA, "دادهٔ سنسور DHT"),
        (C_PWM, "ورودی PWM فن چهار سیمه"),
        (C_SDA, "SDA نمایشگر I2C"),
        (C_SCL, "SCL نمایشگر I2C"),
        (C_TACH, "خروجی تاخ (RPM)"),
    ])
    s.note(825, 30, "تذکر: فن چهار سیمه به‌صورت مستقیم به پین PWM متصل می‌شود (بدون MOSFET). فرکانس PWM روی ۲۵kHz تنظیم می‌شود.",
            color="#7a3e00", size=10.5)
    s.save("schematic-4wire.png")


if __name__ == "__main__":
    draw_default()
    draw_lm35()
    draw_4wire()
