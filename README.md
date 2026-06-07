# Electronic paper driver (Pico RP2040)

Small e-paper (EPD) driver and graphics canvas for Raspberry Pi Pico / RP2040 using the Pico SDK. Currently targets a 2.13" panel via `src/epd_2_13`.

## Features
- **Orientation Support:** Native rotation handling (0°, 90°, 180°, 270°).
- **Graphics Primitives:** Lines, rectangles (filled/unfilled), circles (filled/unfilled).
- **Text Rendering:** Support for proportional and monospace fonts (Tahoma, Cambria, Noto Kufi Arabic). Includes background rendering.
- **Image Rendering:** Draw 1-bit / 2-bit arbitrary bitmaps.
- **Double Buffering:** Draws onto an in-memory canvas before sending to the display to minimize flashing.

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
    uint8_t canvas[EPD_2_13_WIDTH * EPD_2_13_HEIGHT / 4] = {0};
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

