# Electronic paper driver (Pico RP2040)

Small e-paper (EPD) driver + graphics canvas for Raspberry Pi Pico / RP2040 (Pico SDK). Currently targets a 2.13" panel via `src/epd_2_13`.

## Supported displays
- 2.13 inch (MH-ET LIVE)

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
