#pragma once

#include "hardware/spi.h"
#include "pico/stdlib.h"

#define EPD_2_13_WIDTH 122
#define EPD_2_13_HEIGHT 250

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