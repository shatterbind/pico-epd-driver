#pragma once

#include "hardware/spi.h"
#include "pico/stdlib.h"

#define EPD_2_13_WIDTH 122
#define EPD_2_13_HEIGHT 250

/**
 * Canvas buffer size in bytes for the default 2-bit depth used by epd_create().
 * Each row is padded to whole bytes (122 px * 2 bit = 31 bytes), so this is
 * NOT WIDTH * HEIGHT / 4. At runtime, epd_canvas_size() gives the same value.
 */
#define EPD_2_13_CANVAS_SIZE (((EPD_2_13_WIDTH * 2 + 7) / 8) * EPD_2_13_HEIGHT)

#ifndef EPD_2_13_PIN_CS
#define EPD_2_13_PIN_CS 9
#endif

#ifndef EPD_2_13_PIN_CLK
#define EPD_2_13_PIN_CLK 10
#endif

#ifndef EPD_2_13_PIN_MOSI
#define EPD_2_13_PIN_MOSI 11
#endif

#ifndef EPD_2_13_PIN_RESET
#define EPD_2_13_PIN_RESET 12
#endif

#ifndef EPD_2_13_PIN_BUSY
#define EPD_2_13_PIN_BUSY 13
#endif

#ifndef EPD_BUSY_DEFAULT_TIMEOUT_MS
#define EPD_BUSY_DEFAULT_TIMEOUT_MS 10000
#endif