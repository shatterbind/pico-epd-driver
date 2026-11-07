#!/usr/bin/env python3
"""
generate_watering_template.py

Generates:
1. assets/watering_template.png (122x250) - Clean background template with icons and UI slots
2. assets/watering_preview.png  (122x250) - Populated with sample values to preview
3. src/canvas/images/watering_template.h - 2bpp packed C header for pico-epd-driver
"""

from PIL import Image, ImageDraw, ImageFont
import os
import math

WIDTH = 122
HEIGHT = 250

# 4-color palette for 2.13" EPD
BLACK  = (0, 0, 0)
WHITE  = (255, 255, 255)
YELLOW = (255, 205, 0)
RED    = (215, 25, 25)

def draw_round_rectangle(draw, xy, radius, outline=None, fill=None, width=1):
    x0, y0, x1, y1 = xy
    draw.rounded_rectangle([x0, y0, x1, y1], radius=radius, fill=fill, outline=outline, width=width)

def draw_sprout_icon(draw, cx, cy):
    # Soil base mound
    draw.arc([cx - 10, cy + 5, cx + 10, cy + 13], 180, 360, fill=BLACK, width=2)
    # Stem
    draw.line([cx, cy + 9, cx, cy - 2], fill=BLACK, width=2)
    
    # Left Leaf (curved teardrop)
    # Leaf polygon
    left_leaf = [
        (cx, cy + 2),
        (cx - 5, cy),
        (cx - 10, cy - 4),
        (cx - 11, cy - 8),
        (cx - 6, cy - 8),
        (cx - 2, cy - 4),
        (cx, cy - 1)
    ]
    draw.polygon(left_leaf, fill=YELLOW, outline=BLACK)
    
    # Right Leaf
    right_leaf = [
        (cx, cy + 1),
        (cx + 6, cy - 2),
        (cx + 11, cy - 7),
        (cx + 12, cy - 11),
        (cx + 7, cy - 10),
        (cx + 2, cy - 5),
        (cx, cy - 2)
    ]
    draw.polygon(right_leaf, fill=RED, outline=BLACK)

def draw_battery_icon(draw, x, y, fill_bars=3):
    # x=76, y=6 (width 16, height 9)
    draw.rectangle([x, y, x + 16, y + 8], outline=BLACK, width=1)
    draw.rectangle([x + 17, y + 2, x + 18, y + 6], fill=BLACK) # positive nipple
    
    # Inner bars (up to 3 bars)
    for i in range(fill_bars):
        bx = x + 2 + i * 4
        draw.rectangle([bx, y + 2, bx + 2, y + 6], fill=YELLOW if fill_bars > 1 else RED)

def draw_droplet_icon(draw, cx, cy):
    # Water droplet
    pts = [
        (cx, cy - 10),       # top tip
        (cx + 3, cy - 5),
        (cx + 7, cy),
        (cx + 7, cy + 5),
        (cx + 5, cy + 9),
        (cx, cy + 11),       # bottom center
        (cx - 5, cy + 9),
        (cx - 7, cy + 5),
        (cx - 7, cy),
        (cx - 3, cy - 5),
    ]
    draw.polygon(pts, fill=YELLOW, outline=BLACK)
    # Highlight inside
    draw.line([cx - 4, cy + 1, cx - 2, cy + 7], fill=WHITE, width=1)

def draw_thermometer_icon(draw, cx, cy):
    # Stem: width 4, top round
    stem_x0 = cx - 2
    stem_x1 = cx + 2
    stem_y0 = cy - 10
    stem_y1 = cy + 3
    
    # Bulb
    bulb_r = 5
    draw.ellipse([cx - bulb_r, cy + 2, cx + bulb_r, cy + 2 + bulb_r * 2], fill=RED, outline=BLACK, width=1)
    draw.rectangle([stem_x0, stem_y0, stem_x1, stem_y1], fill=WHITE, outline=BLACK, width=1)
    
    # Red liquid column in stem
    draw.line([cx, stem_y0 + 3, cx, stem_y1 + 2], fill=RED, width=2)
    
    # Ticks on right
    for ty in [stem_y0 + 2, stem_y0 + 6, stem_y0 + 10]:
        draw.line([stem_x1 + 1, ty, stem_x1 + 3, ty], fill=BLACK, width=1)

def draw_clock_icon(draw, cx, cy, r=6):
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], outline=BLACK, width=1, fill=WHITE)
    draw.line([cx, cy, cx, cy - 4], fill=BLACK, width=1) # 12 o'clock
    draw.line([cx, cy, cx + 3, cy + 1], fill=BLACK, width=1) # ~2 o'clock
    draw.point([cx, cy], fill=RED)

def create_watering_image(preview=False):
    img = Image.new('RGB', (WIDTH, HEIGHT), WHITE)
    draw = ImageDraw.Draw(img)
    
    # Font setup
    try:
        font_tahoma_bold_12 = ImageFont.truetype("tahoma.ttf", 12)
        font_tahoma_8 = ImageFont.truetype("tahoma.ttf", 9)
        font_tahoma_bold_8 = ImageFont.truetype("tahomabd.ttf", 9)
        font_tahoma_16 = ImageFont.truetype("tahoma.ttf", 15)
        font_tahoma_bold_18 = ImageFont.truetype("tahomabd.ttf", 18)
    except:
        font_tahoma_bold_12 = ImageFont.load_default()
        font_tahoma_8 = ImageFont.load_default()
        font_tahoma_bold_8 = ImageFont.load_default()
        font_tahoma_16 = ImageFont.load_default()
        font_tahoma_bold_18 = ImageFont.load_default()

    # -------------------------------------------------------------
    # 1. TOP HEADER / STATUS BAR (y: 2 to 20)
    # -------------------------------------------------------------
    # Plant watering badge icon & text
    draw.text((6, 5), "IRRIGO", fill=BLACK, font=font_tahoma_bold_8)
    draw.rectangle([46, 7, 50, 11], fill=RED) # status dot
    
    # Battery Symbol (x=66, y=6)
    draw_battery_icon(draw, 66, 6, fill_bars=3 if preview else 0)
    
    if preview:
        draw.text((88, 4), "3.85V", fill=BLACK, font=font_tahoma_8)

    # Top divider
    draw.line([4, 20, 117, 20], fill=BLACK, width=1)

    # -------------------------------------------------------------
    # 2. HERO CARD: PLANT IDENTITY (y: 24 to 76)
    # -------------------------------------------------------------
    draw_round_rectangle(draw, [4, 24, 117, 76], radius=4, outline=BLACK, width=1)
    draw.text((10, 27), "PLANT", fill=BLACK, font=font_tahoma_bold_8)

    # Sprout illustration in top right of card
    draw_sprout_icon(draw, 96, 44)

    # Plant Name input slot
    if preview:
        draw.text((12, 38), "MONSTERA", fill=BLACK, font=font_tahoma_bold_12)
        draw.text((12, 54), "DELICIOSA", fill=RED, font=font_tahoma_bold_8)
        draw.text((12, 64), "Zone A - Living room", fill=BLACK, font=font_tahoma_8)

    # -------------------------------------------------------------
    # 3. METRIC CARD 1: MOISTURE / HUMIDITY (y: 81 to 143)
    # -------------------------------------------------------------
    draw_round_rectangle(draw, [4, 81, 117, 143], radius=4, outline=BLACK, width=1)
    draw.text((10, 84), "SOIL MOISTURE", fill=BLACK, font=font_tahoma_bold_8)

    # Water Droplet Icon
    draw_droplet_icon(draw, 22, 106)

    # Moisture dynamic value
    if preview:
        draw.text((42, 92), "68", fill=BLACK, font=font_tahoma_bold_18)
        draw.text((70, 94), "%", fill=RED, font=font_tahoma_bold_12)
        draw.text((42, 112), "STATUS: OPTIMAL", fill=BLACK, font=font_tahoma_8)

    # Progress / Level Bar (y: 125 to 133)
    bar_x0, bar_y0, bar_x1, bar_y1 = 12, 126, 109, 133
    draw.rectangle([bar_x0, bar_y0, bar_x1, bar_y1], outline=BLACK, fill=WHITE, width=1)
    
    # Tick marks for DRY, GOOD, WET
    # 33% = 12 + 32 = 44, 66% = 12 + 64 = 76
    draw.line([44, bar_y0, 44, bar_y1], fill=BLACK, width=1)
    draw.line([76, bar_y0, 76, bar_y1], fill=BLACK, width=1)
    
    if preview:
        # Fill up to 68% (approx x=78)
        draw.rectangle([bar_x0 + 1, bar_y0 + 1, 78, bar_y1 - 1], fill=YELLOW)
    
    draw.text((12, 135), "DRY", fill=BLACK, font=font_tahoma_8)
    draw.text((54, 135), "OK", fill=BLACK, font=font_tahoma_8)
    draw.text((93, 135), "WET", fill=BLACK, font=font_tahoma_8)

    # -------------------------------------------------------------
    # 4. METRIC CARD 2: TEMPERATURE (y: 149 to 201)
    # -------------------------------------------------------------
    draw_round_rectangle(draw, [4, 149, 117, 201], radius=4, outline=BLACK, width=1)
    draw.text((10, 152), "TEMPERATURE", fill=BLACK, font=font_tahoma_bold_8)

    # Thermometer icon
    draw_thermometer_icon(draw, 22, 175)

    # Temperature dynamic value
    if preview:
        draw.text((42, 162), "24.5", fill=BLACK, font=font_tahoma_bold_18)
        draw.text((86, 164), "\u00B0C", fill=RED, font=font_tahoma_bold_12)
        draw.text((42, 184), "AMBIENT TEMP", fill=BLACK, font=font_tahoma_8)

    # -------------------------------------------------------------
    # 5. FOOTER: REFRESH TIMESTAMP & BRANDING (y: 207 to 247)
    # -------------------------------------------------------------
    draw_round_rectangle(draw, [4, 207, 117, 246], radius=4, outline=BLACK, width=1)
    
    # Clock Icon
    draw_clock_icon(draw, 18, 226, r=6)

    draw.text((30, 212), "LAST REFRESH", fill=BLACK, font=font_tahoma_bold_8)
    
    if preview:
        draw.text((30, 224), "2026-11-07 08:30", fill=RED, font=font_tahoma_bold_8)
        draw.text((30, 235), "Next check in 4h", fill=BLACK, font=font_tahoma_8)

    # Corner botanical micro-accents
    draw.point([2, 2], fill=BLACK)
    draw.point([WIDTH - 3, 2], fill=BLACK)
    draw.point([2, HEIGHT - 3], fill=BLACK)
    draw.point([WIDTH - 3, HEIGHT - 3], fill=BLACK)

    return img

def main():
    os.makedirs("assets", exist_ok=True)
    os.makedirs("src/canvas/images", exist_ok=True)

    template = create_watering_image(preview=False)
    template.save("assets/watering_template.png")
    print("Saved assets/watering_template.png (122x250)")

    preview = create_watering_image(preview=True)
    preview.save("assets/watering_preview.png")
    print("Saved assets/watering_preview.png (122x250)")

if __name__ == "__main__":
    main()

