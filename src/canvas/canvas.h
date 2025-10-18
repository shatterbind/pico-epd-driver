#pragma once

#include <stdlib.h>
#include "epd.h"

#define START_CHAR 32
#define END_CHAR 126

#define USE_FONT_CAMBRIA_16

#define USE_FONT_TAHOMA_8
#define USE_FONT_TAHOMA_12
#define USE_FONT_TAHOMA_16
#define USE_FONT_TAHOMA_24

#define USE_NOTO_KUFI_ARABIC_8
#define USE_NOTO_KUFI_ARABIC_12

#ifdef USE_NOTO_KUFI_ARABIC_8
#include "noto_kufi_arabic_8_font.h"
#endif

#ifdef USE_NOTO_KUFI_ARABIC_12
#include "noto_kufi_arabic_12_font.h"
#endif

#ifdef USE_FONT_CAMBRIA_16
#include "font_cambria_16.h"
#endif

#ifdef USE_FONT_TAHOMA_8
#include "tahoma_8_font.h"
#endif

#ifdef USE_FONT_TAHOMA_12
#include "tahoma_12_font.h"
#endif

#ifdef USE_FONT_TAHOMA_16
#include "tahoma_16_font.h"
#endif

#ifdef USE_FONT_TAHOMA_24
#include "tahoma_24_font.h"
#endif

/*************************************/
/*         TYPE DEFENITION           */
/*************************************/
typedef enum
{
    BLACK = 0,
    WHITE = 1,
    YELLOW = 2,
    RED = 3,
} color_t;

/*************************************/
/*           ENUMERATION             */
/*************************************/
typedef enum
{
    FONT_CAMBRIA_16,
    FONT_TAHOMA_8,
    FONT_TAHOMA_16,
    FONT_TAHOMA_12,
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
/*      FUNCION DECLARATION          */
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
/*      FUNCION DECLARATION          */
/*************************************/
void fill_background(epd_t *epd, color_t color, uint8_t *canvas);
void epd_set_pixel(epd_t *epd, uint8_t *canvas, uint16_t x, uint16_t y, uint8_t color);
void draw_text(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, uint8_t gap, uint8_t *canvas);
void draw_text_with_bg(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, color_t background);

void draw_line(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, color_t color);

void draw_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color);
void fill_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color, uint8_t *canvas);

void draw_circle(epd_t *epd, uint16_t x_center, uint16_t y_center, uint16_t radius, color_t color);
void fill_circle(epd_t *epd, uint16_t x_center, uint16_t y_center, uint16_t radius, color_t color);

void draw_image(epd_t *epd, uint16_t x_start, uint16_t y_start, const uint8_t *image_data, uint16_t width, uint16_t height);

font_property_t font_properties(text_type_t type);