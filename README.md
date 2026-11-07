# Electronic paper driver (Pico RP2040 & RP2350)

Small e-paper (EPD) driver and graphics canvas for Raspberry Pi Pico & Pico 2 (RP2040 / RP2350) using the Pico SDK. Currently targets a 2.13" panel via `src/epd_2_13`.

## Features
- **Orientation Support:** Native rotation handling (0°, 90°, 180°, 270°).
- **Graphics Primitives:** Lines, rectangles (filled/unfilled), rounded rectangles (filled/unfilled), circles (filled/unfilled), triangles (filled/unfilled).
- **Text Rendering & Metrics:** Proportional and monospace fonts (Tahoma, Cambria, Noto Kufi Arabic). Includes background rendering and text bounding box measurement (`canvas_get_text_width`, `canvas_get_text_height`).
- **Image Rendering:** Draw 1-bit / 2-bit arbitrary bitmaps.
- **Double Buffering:** Draws onto an in-memory canvas before sending to the display to minimize flashing.
- **High Reliability:** Configurable SPI baud rate (up to 4–10 MHz) and hardware busy pin timeout handling.

## Supported displays
- 2.13 inch (MH-ET LIVE) - 124x250 typically (via SPI).

## Connection (2.13" panel)

Default pin mapping (override via `EPD_2_13_PIN_*` macros in `src/epd_2_13/epd_2_13.h`):

| EPD signal  |    Pico GPIO | Notes                                                   |
| ----------- | -----------: | ------------------------------------------------------- |
| `CS`        |          `9` | SPI chip-select                                         |
| `SCK/CLK`   |         `10` | SPI1 SCK                                                |
| `MOSI/DIN`  |         `11` | SPI1 TX                                                 |
| `RST/RESET` |         `12` | Reset                                                   |
| `BUSY`      |         `13` | Busy (input)                                            |
| `DC`        | user-defined | Passed to `epd_create(dc_gpio)` (example uses GPIO `8`) |
| `VCC`       |        `3V3` | Power (per your panel’s spec)                           |
| `GND`       |        `GND` | Ground                                                  |

Notes:
- `MISO` is not used.
- The Pico is 3.3V logic; follow your panel vendor’s guidance for power/level shifting if needed.

## Building and Running

1. Make sure you have the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) installed and configured.
2. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
3. Copy the generated `.uf2` file to the `RPI-RP2` mass storage device.

## Usage Example

```c
#include "pico/stdlib.h"
#include "epd.h"
#include "canvas.h"
#include "epd_2_13.h"

int main() {
    stdio_init_all();

    // 1. Initialize Display
    epd_t epd = epd_create(8); // 8 is the DC pin
    epd_init(&epd);
    epd.rotation = EPD_ROT_270; // Set rotation

    // 2. Prepare Canvas
    static uint8_t canvas[EPD_2_13_CANVAS_SIZE] = {0}; // rows are byte-padded: not WIDTH*HEIGHT/4
    fill_background(&epd, WHITE, canvas);

    // 3. Draw Graphics
    draw_text(&epd, 10, 10, "Hello World", FONT_TAHOMA_16, BLACK, 2, canvas);
    draw_rect(&epd, 5, 5, 100, 30, RED, canvas);

    // 4. Update Display
    epd_display(&epd, canvas);
    epd_sleep(&epd);

    while (true) {
        sleep_ms(1000);
    }
}
```

## Watering System UI Template (122 × 250 Vertical)

A dedicated 4-color portrait template for a plant watering / soil moisture monitoring system is provided in `assets/watering_template.png` and compiled as a C header in `src/canvas/images/watering_template.h`.

All dynamic sensor and identity fields are designed as pure white empty regions so you can overlay live data directly using `draw_text()` or `draw_text_with_bg()`.

### Card Containers & Border Layout

The display canvas has a logical resolution of **122 × 250 pixels** (`EPD_ROT_0` or `EPD_ROT_180`).

| Section | Coordinates `(X0, Y0) – (X1, Y1)` | Dimensions `(W × H)` | Corner Radius | Elements / Badges |
| :--- | :---: | :---: | :---: | :--- |
| **Header Status Bar** | `(4, 2) – (117, 20)` | 114 × 19 px | — | "IRRIGO" branding, status LED dot, battery outline `(66, 6) [17×9 px]`, divider line at `Y=20` |
| **Plant Hero Card** | `(4, 24) – (117, 76)` | 114 × 53 px | 4 px | "PLANT" label at `(10, 27)`, Sprout icon centered at `(96, 44)` |
| **Moisture / Humidity Card** | `(4, 81) – (117, 143)` | 114 × 63 px | 4 px | "SOIL MOISTURE" label at `(10, 84)`, Droplet icon at `(22, 106)`, Gauge bar outline at `Y=126` |
| **Temperature Card** | `(4, 149) – (117, 201)` | 114 × 53 px | 4 px | "TEMPERATURE" label at `(10, 152)`, Thermometer icon at `(22, 175)` |
| **Refresh / Footer Card** | `(4, 207) – (117, 246)` | 114 × 40 px | 4 px | "LAST REFRESH" label at `(30, 212)`, Clock icon centered at `(18, 226, R=6)` |

---

### Dynamic Text Fields & Typography Specifications

| Field | Start `(X, Y)` | Available Area `(W × H)` | Recommended Font | Font Metrics (H × Max W) | Color | Example Text |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| **Battery Voltage** | `(86, 4)` | 30 × 12 px | `FONT_TAHOMA_8` | 11 px H × 10 px W | `BLACK` | `3.85V` |
| **Plant Name** | `(12, 38)` | 72 × 16 px | `FONT_TAHOMA_12` | 16 px H × 14 px W | `BLACK` | `MONSTERA` |
| **Plant Subtitle / Zone** | `(12, 56)` | 72 × 12 px | `FONT_TAHOMA_8` | 11 px H × 10 px W | `RED` | `DELICIOSA` / `Zone A` |
| **Soil Moisture Value** | `(42, 94)` | 68 × 22 px | `FONT_TAHOMA_16` | 21 px H × 19 px W | `BLACK` | `68%` |
| **Moisture Status Label** | `(42, 112)` | 68 × 12 px | `FONT_TAHOMA_8` | 11 px H × 10 px W | `RED` / `BLACK` | `OPTIMAL` / `NEEDS WATER` |
| **Temperature Value** | `(42, 166)` | 68 × 22 px | `FONT_TAHOMA_16` | 21 px H × 19 px W | `BLACK` | `24.5 C` |
| **Last Refresh Timestamp** | `(30, 224)` | 82 × 14 px | `FONT_TAHOMA_8` | 11 px H × 10 px W | `RED` | `11/07 08:30` |

---

### Moisture Level Gauge Bar Detail

The soil moisture card includes an outline gauge with ticks for **DRY**, **OK**, and **WET**:

- **Bar Outline Bounds**: `(12, 126)` to `(109, 133)` (total length: 98 px, height: 8 px)
- **Fillable Interior Area**: `X = 13` to `108`, `Y = 127` to `132` (max width: 95 px, height: 6 px)
- **Fill Calculation**:
  ```c
  uint16_t fill_w = (uint16_t)(95 * moisture_pct / 100);
  if (fill_w > 95) fill_w = 95;
  fill_rect(&epd, 13, 127, fill_w, 6, YELLOW, canvas);
  ```
- **Tick Marks**: `X = 44` (33% threshold), `X = 76` (66% threshold)
- **Labels (Tahoma 8)**: `"DRY"` at `(12, 133)`, `"OK"` at `(54, 133)`, `"WET"` at `(93, 133)` (leaving 1px padding above and below)

---

### C Overlay Example

```c
#include "canvas.h"
#include "images/watering_template.h"

void render_watering_screen(epd_t *epd, uint8_t *canvas, const char *plant_name,
                            float battery_v, int moisture_pct, float temp_c, const char *timestamp) {
    // 1. Draw static template background (cards, icons, frames)
    draw_fullscreen_image(epd, watering_template_122x250, WATERING_TEMPLATE_WIDTH, WATERING_TEMPLATE_HEIGHT, canvas);

    char buf[32];

    // 2. Battery Voltage
    snprintf(buf, sizeof(buf), "%.2fV", battery_v);
    draw_text_with_bg(epd, 86, 4, buf, FONT_TAHOMA_8, BLACK, 1, WHITE, canvas);

    // 3. Plant Name & Subtitle
    draw_text_with_bg(epd, 12, 38, plant_name, FONT_TAHOMA_12, BLACK, 1, WHITE, canvas);
    draw_text_with_bg(epd, 12, 56, "Zone A", FONT_TAHOMA_8, RED, 1, WHITE, canvas);

    // 4. Moisture percentage and filled gauge bar
    snprintf(buf, sizeof(buf), "%d%%", moisture_pct);
    draw_text_with_bg(epd, 42, 94, buf, FONT_TAHOMA_16, BLACK, 1, WHITE, canvas);
    draw_text_with_bg(epd, 42, 112, (moisture_pct < 30) ? "DRY" : "OPTIMAL", FONT_TAHOMA_8, RED, 1, WHITE, canvas);

    uint16_t fill_w = (uint16_t)(95 * moisture_pct / 100);
    if (fill_w > 95) fill_w = 95;
    fill_rect(epd, 13, 127, fill_w, 6, YELLOW, canvas);

    // 5. Temperature
    snprintf(buf, sizeof(buf), "%.1f C", temp_c);
    draw_text_with_bg(epd, 42, 166, buf, FONT_TAHOMA_16, BLACK, 1, WHITE, canvas);

    // 6. Last Refresh
    draw_text_with_bg(epd, 30, 224, timestamp, FONT_TAHOMA_8, RED, 1, WHITE, canvas);

    // 7. Push to display
    epd_display(epd, canvas);
}
```


