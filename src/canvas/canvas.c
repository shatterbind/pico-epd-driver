#include "canvas.h"
#include "epd_2_13.h"

static inline bool is_not_printable(char c, font_property_t font_property)
{
    return c < font_property.start || c > font_property.end;
}

void epd_set_pixel(epd_t *epd, uint8_t *canvas, uint16_t x, uint16_t y, uint8_t color)
{
    uint8_t bits_per_pixel = epd->depth;
    uint8_t pixels_per_byte = 8 / bits_per_pixel;

    uint32_t index = (y * epd->width + x) / pixels_per_byte;
    uint8_t shift = (pixels_per_byte - 1 - (x % pixels_per_byte)) * bits_per_pixel;

    uint8_t mask = ((1 << bits_per_pixel) - 1) << shift;
    canvas[index] = (canvas[index] & ~mask) | ((color & ((1 << bits_per_pixel) - 1)) << shift);
}

void fill_background(epd_t *epd, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas)
        return;

    for (uint16_t i = 0; i < epd->height * epd->width / sizeof(uint8_t) / epd->depth; i++)
    {
        canvas[i] = (color << 6) | (color << 4) | (color << 2) | color;
    }
}

void draw_text(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, uint8_t gap, uint8_t *canvas)
{
    if (!epd || !text || !canvas)
        return;

    const font_property_t font = font_properties(type);
    const uint8_t mask = 0b00000001;

    uint32_t offset_array = 0;
    uint8_t offset_bit = 0;
    uint16_t width = font.width;

    for (const char *p = text; *p != '\0'; ++p)
    {
        if (is_not_printable(*p, font))
            continue;

        offset_array = (*p - font.start) * font.length;
        uint32_t offset_row = 0;

        if (font.type == proportional)
        {
            width = font.pointer[offset_array];
            offset_array++;
        }
        else
        {
            width = font.width;
        }

        for (uint16_t row = 0; row < font.height; row++)
        {
            offset_bit = 0;
            uint32_t temp_offset_row = offset_row;

            for (uint16_t col = 0; col < width; col++)
            {
                if ((font.pointer[offset_array + temp_offset_row] >> offset_bit) & mask)
                {
                    epd_set_pixel(epd, canvas, x + col, y + row, color);
                }

                offset_bit++;

                if (offset_bit == 8)
                {
                    offset_bit = 0;
                    temp_offset_row++;
                }
            }

            offset_row += (font.width + 7) / 8;
        }

        x += width + gap;
    }
}

// void draw_text_with_bg(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, color_t background)
// {
//     if (!epd || !text)
//     {
//         return;
//     }

//     font_property_t font = font_properties(type);

//     const uint16_t length = font.width * font.height * 2;
//     uint8_t data[length];

//     uint8_t color_high = (uint8_t)(color >> 8);
//     uint8_t color_low = (uint8_t)color;

//     uint8_t bg_high = (uint8_t)(background >> 8);
//     uint8_t bg_low = (uint8_t)background;

//     uint32_t offset_array = 0;
//     uint32_t offset_row = 0;
//     uint8_t offset_bit = 0;

//     for (const char *p = text; *p != '\0'; p++)
//     {
//         if (is_not_printable(*p, font))
//         {
//             continue;
//         }

//         offset_array = (*p - 32) * font.length;
//         offset_row = 0;
//         offset_bit = 0;

//         for (uint16_t row = 0; row < font.height; row++)
//         {
//             for (uint16_t col = 0; col < font.width; col++)
//             {
//                 if (col % 8 == 0 && col != 0)
//                 {
//                     offset_bit = 0;
//                     offset_row++;
//                 }

//                 if ((font.pointer[offset_array + offset_row] >> offset_bit) & 0b00000001)
//                 {
//                     data[(row * font.width + col) * 2] = color_high;
//                     data[(row * font.width + col) * 2 + 1] = color_low;
//                 }
//                 else
//                 {
//                     data[(row * font.width + col) * 2] = bg_high;
//                     data[(row * font.width + col) * 2 + 1] = bg_low;
//                 }

//                 offset_bit++;
//             }
//             offset_row++;
//             offset_bit = 0;
//         }

//         epd_set_window(epd, x, y, x + font.width - 1, y + font.height - 1);
//         epd_flush(epd, data, length);

//         x += font.width + font.width / 8;
//     }
// }

// void draw_line(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, color_t color)
// {
//     if (!epd)
//     {
//         return;
//     }

//     int16_t dx = abs(x1 - x0);
//     int16_t dy = abs(y1 - y0);
//     int16_t sx = (x0 < x1) ? 1 : -1;
//     int16_t sy = (y0 < y1) ? 1 : -1;
//     int16_t err = dx - dy;

//     int16_t length = (dx > dy) ? dx + 1 : dy + 1;
//     uint8_t buffer[length * 2];

//     uint8_t color_high = (uint8_t)(color >> 8);
//     uint8_t color_low = (uint8_t)color;

//     for (int16_t i = 0; i < length; i++)
//     {
//         buffer[i * 2] = color_high;
//         buffer[i * 2 + 1] = color_low;
//     }

//     if (dx == 0 || dy == 0)
//     {
//         if (dy == 0)
//         {
//             epd_set_window(epd, x0, y0, x1, y1);
//         }
//         else
//         {

//             epd_set_window(epd, x0, y0, x0, y1);
//         }

//         epd_flush(epd, buffer, length * 2);
//         return;
//     }

//     for (int16_t i = 0; i < length; ++i)
//     {
//         epd_draw_pixel(epd, x0, y0, color);

//         int16_t e2 = err * 2;
//         if (e2 > -dy)
//         {
//             err -= dy;
//             x0 += sx;
//         }
//         if (e2 < dx)
//         {
//             err += dx;
//             y0 += sy;
//         }
//     }
// }

// void draw_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color)
// {
//     if (!epd || width == 0 || height == 0)
//     {
//         return;
//     }

//     draw_line(epd, x, y, x + width - 1, y, color);
//     draw_line(epd, x, y + height - 1, x + width - 1, y + height - 1, color);
//     draw_line(epd, x, y, x, y + height - 1, color);
//     draw_line(epd, x + width - 1, y, x + width - 1, y + height - 1, color);
// }

void fill_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color, uint8_t *canvas)
{
    if (!epd)
    {
        return;
    }

    for (uint16_t i = y; i < y + height; i++)
    {
        for (uint16_t j = x; j < x + width; j++)
        {
            if (i > epd->height || j > epd->width)
                continue;

            epd_set_pixel(epd, canvas, j, i, color);
        }
    }
}

// inline void draw_circle_points(epd_t *epd, int xc, int yc, int x, int y, color_t color)
// {
//     epd_draw_pixel(epd, xc + x, yc + y, color);
//     epd_draw_pixel(epd, xc - x, yc + y, color);
//     epd_draw_pixel(epd, xc + x, yc - y, color);
//     epd_draw_pixel(epd, xc - x, yc - y, color);
//     epd_draw_pixel(epd, xc + y, yc + x, color);
//     epd_draw_pixel(epd, xc - y, yc + x, color);
//     epd_draw_pixel(epd, xc + y, yc - x, color);
//     epd_draw_pixel(epd, xc - y, yc - x, color);
// }

// void draw_circle(epd_t *epd, uint16_t x_center, uint16_t y_center, uint16_t radius, color_t color)
// {
//     int x = 0, y = radius;
//     int d = 3 - 2 * radius;

//     while (x <= y)
//     {
//         draw_circle_points(epd, x_center, y_center, x, y, color);

//         if (d > 0)
//         {
//             y--;
//             d = d + 4 * (x - y) + 10;
//         }
//         else
//         {
//             d = d + 4 * x + 6;
//         }
//         x++;
//     }
// }

// void fill_circle(epd_t *epd, uint16_t x_center, uint16_t y_center, uint16_t radius, color_t color)
// {
//     if (!epd || radius == 0)
//     {
//         return;
//     }

//     int16_t x = radius;
//     int16_t y = 0;
//     int16_t radius_error = 1 - x;

//     while (x >= y)
//     {
//         for (int16_t i = x_center - x; i <= x_center + x; i++)
//         {
//             epd_draw_pixel(epd, i, y_center + y, color);
//             epd_draw_pixel(epd, i, y_center - y, color);
//         }

//         for (int16_t i = x_center - y; i <= x_center + y; i++)
//         {
//             epd_draw_pixel(epd, i, y_center + x, color);
//             epd_draw_pixel(epd, i, y_center - x, color);
//         }

//         y++;

//         if (radius_error < 0)
//         {
//             radius_error += 2 * y + 1;
//         }
//         else
//         {
//             x--;
//             radius_error += 2 * (y - x + 1);
//         }
//     }
// }

font_property_t font_properties(text_type_t type)
{
    font_property_t font_property;

    switch (type)
    {

#ifdef USE_FONT_CAMBRIA_16
    case FONT_CAMBRIA_16:
        font_property.pointer = font_cambria_16;
        font_property.width = FONT_CAMBRIA_16_CHAR_WIDTH;
        font_property.height = FONT_CAMBRIA_16_CHAR_HEIGHT;
        font_property.length = FONT_CAMBRIA_16_ARRAY_LENGTH / FONT_CAMBRIA_16_LENGTH;
        font_property.type = FONT_CAMBRIA_16_FONT_TYPE;
        font_property.start = FONT_CAMBRIA_16_START_CHAR;
        font_property.end = FONT_CAMBRIA_16_START_CHAR + FONT_CAMBRIA_16_LENGTH;
        break;
#endif

#ifdef USE_FONT_TAHOMA_8
    case FONT_TAHOMA_8:
        font_property.pointer = font_tahoma_8;
        font_property.width = FONT_TAHOMA_8_CHAR_WIDTH;
        font_property.height = FONT_TAHOMA_8_CHAR_HEIGHT;
        font_property.length = FONT_TAHOMA_8_ARRAY_LENGTH / FONT_TAHOMA_8_LENGTH;
        font_property.type = FONT_TAHOMA_8_FONT_TYPE;
        font_property.start = FONT_TAHOMA_8_START_CHAR;
        font_property.end = FONT_TAHOMA_8_START_CHAR + FONT_TAHOMA_8_LENGTH;
        break;
#endif

#ifdef USE_FONT_TAHOMA_12
    case FONT_TAHOMA_12:
        font_property.pointer = font_tahoma_12;
        font_property.width = FONT_TAHOMA_12_CHAR_WIDTH;
        font_property.height = FONT_TAHOMA_12_CHAR_HEIGHT;
        font_property.length = FONT_TAHOMA_12_ARRAY_LENGTH / FONT_TAHOMA_12_LENGTH;
        font_property.type = FONT_TAHOMA_12_FONT_TYPE;
        font_property.start = FONT_TAHOMA_12_START_CHAR;
        font_property.end = FONT_TAHOMA_12_START_CHAR + FONT_TAHOMA_12_LENGTH;
        break;
#endif

#ifdef USE_FONT_TAHOMA_16
    case FONT_TAHOMA_16:
        font_property.pointer = font_tahoma_16;
        font_property.width = FONT_TAHOMA_16_CHAR_WIDTH;
        font_property.height = FONT_TAHOMA_16_CHAR_HEIGHT;
        font_property.length = FONT_TAHOMA_16_ARRAY_LENGTH / FONT_TAHOMA_16_LENGTH;
        font_property.type = FONT_TAHOMA_16_FONT_TYPE;
        font_property.start = FONT_TAHOMA_16_START_CHAR;
        font_property.end = FONT_TAHOMA_16_START_CHAR + FONT_TAHOMA_16_LENGTH;
        break;
#endif

#ifdef USE_FONT_TAHOMA_24
    case FONT_TAHOMA_24:
        font_property.pointer = font_tahoma_24;
        font_property.width = FONT_TAHOMA_24_CHAR_WIDTH;
        font_property.height = FONT_TAHOMA_24_CHAR_HEIGHT;
        font_property.length = FONT_TAHOMA_24_ARRAY_LENGTH / FONT_TAHOMA_24_LENGTH;
        font_property.type = FONT_TAHOMA_24_FONT_TYPE;
        font_property.start = FONT_TAHOMA_24_START_CHAR;
        font_property.end = FONT_TAHOMA_24_START_CHAR + FONT_TAHOMA_24_LENGTH;
        break;
#endif

#ifdef USE_NOTO_KUFI_ARABIC_8
    case NOTO_KUFI_ARABIC_8:
        font_property.pointer = font_noto_kufi_arabic_8;
        font_property.width = FONT_NOTO_KUFI_ARABIC_8_CHAR_WIDTH;
        font_property.height = FONT_NOTO_KUFI_ARABIC_8_CHAR_HEIGHT;
        font_property.length = FONT_NOTO_KUFI_ARABIC_8_ARRAY_LENGTH / FONT_NOTO_KUFI_ARABIC_8_LENGTH;
        font_property.type = FONT_NOTO_KUFI_ARABIC_8_FONT_TYPE;
        font_property.start = FONT_NOTO_KUFI_ARABIC_8_START_CHAR;
        font_property.end = FONT_NOTO_KUFI_ARABIC_8_START_CHAR + FONT_NOTO_KUFI_ARABIC_8_LENGTH;
        break;
#endif

#ifdef USE_NOTO_KUFI_ARABIC_12
    case NOTO_KUFI_ARABIC_12:
        font_property.pointer = font_noto_kufi_arabic_12;
        font_property.width = FONT_NOTO_KUFI_ARABIC_12_CHAR_WIDTH;
        font_property.height = FONT_NOTO_KUFI_ARABIC_12_CHAR_HEIGHT;
        font_property.length = FONT_NOTO_KUFI_ARABIC_12_ARRAY_LENGTH / FONT_NOTO_KUFI_ARABIC_12_LENGTH;
        font_property.type = FONT_NOTO_KUFI_ARABIC_12_FONT_TYPE;
        font_property.start = FONT_NOTO_KUFI_ARABIC_12_START_CHAR;
        font_property.end = FONT_NOTO_KUFI_ARABIC_12_START_CHAR + FONT_NOTO_KUFI_ARABIC_12_LENGTH;
        break;
#endif

    }

    return font_property;
}

// void draw_image(epd_t *epd, uint16_t x_start, uint16_t y_start, const uint8_t *image_data, uint16_t width, uint16_t height)
// {
//     if (!epd || !image_data || width == 0 || height == 0)
//     {
//         return;
//     }

//     epd_set_window(epd, 0, 0, width - 1, height - 1);
//     epd_flush(epd, image_data, height * width * 2);
// }
