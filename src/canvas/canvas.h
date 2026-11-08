#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include "epd.h"

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/*
 * Font selection. By default every font is compiled in. To build only the
 * fonts you use (saves flash), define CANVAS_SELECT_FONTS plus the USE_* macros
 * you need on the canvas target, e.g.:
 *   target_compile_definitions(canvas PUBLIC CANVAS_SELECT_FONTS USE_FONT_TAHOMA_8)
 */
#ifndef CANVAS_SELECT_FONTS

#ifndef USE_FONT_CAMBRIA_16
#define USE_FONT_CAMBRIA_16
#endif

#ifndef USE_FONT_TAHOMA_8
#define USE_FONT_TAHOMA_8
#endif

#ifndef USE_FONT_TAHOMA_12
#define USE_FONT_TAHOMA_12
#endif

#ifndef USE_FONT_TAHOMA_16
#define USE_FONT_TAHOMA_16
#endif

#ifndef USE_FONT_TAHOMA_24
#define USE_FONT_TAHOMA_24
#endif

#ifndef USE_NOTO_KUFI_ARABIC_8
#define USE_NOTO_KUFI_ARABIC_8
#endif

#ifndef USE_NOTO_KUFI_ARABIC_12
#define USE_NOTO_KUFI_ARABIC_12
#endif

#endif /* CANVAS_SELECT_FONTS */


/*************************************/
/*          TYPE DEFINITION          */
/*************************************/
typedef enum
{
    BLACK = 0,
    WHITE = 1,
    YELLOW = 2,
    RED = 3,
} color_t;

/*************************************/
/*            ENUMERATION            */
/*************************************/
typedef enum
{
    FONT_CAMBRIA_16,
    FONT_TAHOMA_8,
    FONT_TAHOMA_12,
    FONT_TAHOMA_16,
    FONT_TAHOMA_24,
    NOTO_KUFI_ARABIC_8,
    NOTO_KUFI_ARABIC_12,
} text_type_t;

typedef enum
{
    monospace,
    proportional
} font_type_t;

/*************************************/
/*        STRUCT DEFINITION          */
/*************************************/
typedef struct
{
    const uint8_t *pointer;
    uint16_t width;
    uint16_t height;
    uint16_t length;
    uint8_t start;
    uint8_t end;
    font_type_t type;
} font_property_t;

/*************************************/
/*       FUNCTION DECLARATION        */
/*************************************/
uint16_t canvas_width(const epd_t *epd);
uint16_t canvas_height(const epd_t *epd);

void fill_background(epd_t *epd, color_t color, uint8_t *canvas);

void epd_set_pixel(epd_t *epd, uint16_t x, uint16_t y, uint8_t color, uint8_t *canvas);

uint16_t canvas_get_text_width(const char *text, text_type_t type, uint8_t gap);
uint16_t canvas_get_text_height(text_type_t type);

void draw_text(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, uint8_t gap, uint8_t *canvas);
void draw_text_with_bg(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, uint8_t gap, color_t background, uint8_t *canvas);

void draw_line(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, color_t color, uint8_t *canvas);

void draw_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color, uint8_t *canvas);
void fill_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color, uint8_t *canvas);

void draw_round_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, color_t color, uint8_t *canvas);
void fill_round_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, color_t color, uint8_t *canvas);

void draw_circle(epd_t *epd, uint16_t x_center, uint16_t y_center, uint16_t radius, color_t color, uint8_t *canvas);
void fill_circle(epd_t *epd, uint16_t x_center, uint16_t y_center, uint16_t radius, color_t color, uint8_t *canvas);

void draw_triangle(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint8_t *canvas);
void fill_triangle(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint8_t *canvas);

void draw_image(epd_t *epd, uint16_t x_start, uint16_t y_start, const uint8_t *image_data, uint16_t width, uint16_t height, uint8_t *canvas);

font_property_t font_properties(text_type_t type);

