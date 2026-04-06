#include "canvas.h"
#include <stdbool.h>
#include <string.h>

static inline bool is_supported_depth(uint8_t bits_per_pixel)
{
    return bits_per_pixel != 0 && (8 % bits_per_pixel) == 0;
}

static inline bool is_not_printable(uint8_t c, font_property_t font_property)
{
    return c < font_property.start || c > font_property.end;
}

void epd_set_pixel(epd_t *epd, uint16_t x, uint16_t y, uint8_t color, uint8_t *canvas)
{
    if (!epd || !canvas)
    {
        return;
    }

    if (x >= epd->width || y >= epd->height)
    {
        return;
    }

    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    if (!is_supported_depth(bits_per_pixel))
    {
        return;
    }

    const uint8_t pixels_per_byte = 8 / bits_per_pixel;
    uint32_t index = (y * epd->width + x) / pixels_per_byte;
    uint8_t shift = (pixels_per_byte - 1 - (x % pixels_per_byte)) * bits_per_pixel;

    uint8_t mask = ((1 << bits_per_pixel) - 1) << shift;
    canvas[index] = (canvas[index] & ~mask) | ((color & ((1 << bits_per_pixel) - 1)) << shift);
}

void fill_background(epd_t *epd, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas)
    {
        return;
    }

    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    if (!is_supported_depth(bits_per_pixel))
    {
        return;
    }

    const uint32_t total_pixels = (uint32_t)epd->width * (uint32_t)epd->height;
    const uint8_t pixels_per_byte = (uint8_t)(CHAR_BIT / bits_per_pixel);
    const uint32_t total_bytes = (total_pixels + pixels_per_byte - 1u) / pixels_per_byte;

    const uint8_t pixel_mask = (uint8_t)((1u << bits_per_pixel) - 1u);
    const uint8_t packed_color = (uint8_t)color & pixel_mask;

    uint8_t packed_byte = 0;
    for (uint8_t i = 0; i < pixels_per_byte; i++)
    {
        const uint8_t shift = (uint8_t)((pixels_per_byte - 1u - i) * bits_per_pixel);
        packed_byte |= (uint8_t)(packed_color << shift);
    }

    memset(canvas, packed_byte, (size_t)total_bytes);
}

static void draw_text_impl(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, uint8_t gap, bool with_background, color_t background, uint8_t *canvas)
{
    if (!epd || !text || !canvas)
        return;

    const font_property_t font = font_properties(type);
    if (!font.pointer || font.width == 0 || font.height == 0 || font.length == 0)
    {
        return;
    }

    const uint8_t mask = 0b00000001;

    uint32_t offset_array = 0;
    uint8_t offset_bit = 0;
    uint16_t width = font.width;

    for (const char *p = text; *p != '\0'; ++p)
    {
        const uint8_t c = (uint8_t)*p;
        if (is_not_printable(c, font))
            continue;

        offset_array = (uint32_t)(c - font.start) * (uint32_t)font.length;
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
                    epd_set_pixel(epd, x + col, y + row, color, canvas);
                }
                else if (with_background)
                {
                    epd_set_pixel(epd, x + col, y + row, background, canvas);
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

        if (with_background)
        {
            for (uint16_t i = 0; i < gap; i++)
            {
                for (uint16_t j = 0; j < font.height; j++)
                {
                    epd_set_pixel(epd, x + width + i, y + j, background, canvas);
                }
            }
        }

        x += width + gap;
    }
}

void draw_text(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, uint8_t gap, uint8_t *canvas)
{
    draw_text_impl(epd, x, y, text, type, color, gap, false, WHITE, canvas);
}

void draw_text_with_bg(epd_t *epd, uint16_t x, uint16_t y, const char *text, text_type_t type, color_t color, uint8_t gap, color_t background, uint8_t *canvas)
{
    draw_text_impl(epd, x, y, text, type, color, gap, true, background, canvas);
}

void draw_line(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas)
    {
        return;
    }

    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);

    int8_t sx = (x0 < x1) ? 1 : -1;
    int8_t sy = (y0 < y1) ? 1 : -1;

    int16_t err = dx - dy;
    int16_t e2;

    uint16_t x = x0;
    uint16_t y = y0;

    while (true)
    {
        if (x < epd->width && y < epd->height)
        {
            epd_set_pixel(epd, x, y, color, canvas);
        }

        if (x == x1 && y == y1)
        {
            break;
        }

        e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

void draw_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color, uint8_t *canvas)
{
    if (!epd || width == 0 || height == 0)
    {
        return;
    }

    draw_line(epd, x, y, x + width - 1, y, color, canvas);                           // Top edge
    draw_line(epd, x, y, x, y + height - 1, color, canvas);                          // Left edge
    draw_line(epd, x + width - 1, y, x + width - 1, y + height - 1, color, canvas);  // Right edge
    draw_line(epd, x, y + height - 1, x + width - 1, y + height - 1, color, canvas); // Bottom edge
}

void fill_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas || width == 0 || height == 0)
    {
        return;
    }

    uint32_t x_end = (uint32_t)x + (uint32_t)width;
    uint32_t y_end = (uint32_t)y + (uint32_t)height;

    if (x_end > epd->width)
    {
        x_end = epd->width;
    }
    if (y_end > epd->height)
    {
        y_end = epd->height;
    }

    for (uint16_t i = y; i < (uint16_t)y_end; i++)
    {
        for (uint16_t j = x; j < (uint16_t)x_end; j++)
        {
            epd_set_pixel(epd, j, i, color, canvas);
        }
    }
}

void draw_circle(epd_t *epd, uint16_t x_center, uint16_t y_center, uint16_t radius, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas || radius == 0)
    {
        return;
    }

    int x = 0, y = radius;
    int d = 3 - 2 * radius;

    while (x <= y)
    {
        epd_set_pixel(epd, x_center + x, y_center + y, color, canvas);
        epd_set_pixel(epd, x_center - x, y_center + y, color, canvas);
        epd_set_pixel(epd, x_center + x, y_center - y, color, canvas);
        epd_set_pixel(epd, x_center - x, y_center - y, color, canvas);
        epd_set_pixel(epd, x_center + y, y_center + x, color, canvas);
        epd_set_pixel(epd, x_center - y, y_center + x, color, canvas);
        epd_set_pixel(epd, x_center + y, y_center - x, color, canvas);
        epd_set_pixel(epd, x_center - y, y_center - x, color, canvas);

        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
        x++;
    }
}

void fill_circle(epd_t *epd, uint16_t x_center, uint16_t y_center, uint16_t radius, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas || radius == 0)
    {
        return;
    }

    int16_t x = radius;
    int16_t y = 0;
    int16_t radius_error = 1 - x;

    while (x >= y)
    {
        for (int16_t i = x_center - x; i <= x_center + x; i++)
        {
            epd_set_pixel(epd, i, y_center + y, color, canvas);
            epd_set_pixel(epd, i, y_center - y, color, canvas);
        }

        for (int16_t i = x_center - y; i <= x_center + y; i++)
        {
            epd_set_pixel(epd, i, y_center + x, color, canvas);
            epd_set_pixel(epd, i, y_center - x, color, canvas);
        }

        y++;

        if (radius_error < 0)
        {
            radius_error += 2 * y + 1;
        }
        else
        {
            x--;
            radius_error += 2 * (y - x + 1);
        }
    }
}

static inline uint8_t get_packed_pixel(const uint8_t *buffer, uint16_t width, uint16_t x, uint16_t y, uint8_t bits_per_pixel)
{
    const uint8_t pixels_per_byte = (uint8_t)(8u / bits_per_pixel);
    const uint32_t index = ((uint32_t)y * (uint32_t)width + (uint32_t)x) / pixels_per_byte;
    const uint8_t shift = (uint8_t)((pixels_per_byte - 1u - (x % pixels_per_byte)) * bits_per_pixel);
    const uint8_t mask = (uint8_t)((1u << bits_per_pixel) - 1u);
    return (uint8_t)((buffer[index] >> shift) & mask);
}

void draw_image(epd_t *epd, uint16_t x_start, uint16_t y_start, const uint8_t *image_data, uint16_t width, uint16_t height, uint8_t *canvas)
{
    if (!epd || !canvas || !image_data || width == 0 || height == 0)
    {
        return;
    }

    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    if (!is_supported_depth(bits_per_pixel))
    {
        return;
    }

    for (uint16_t y = 0; y < height; y++)
    {
        const uint32_t dst_y = (uint32_t)y_start + (uint32_t)y;
        if (dst_y >= epd->height)
        {
            break;
        }

        for (uint16_t x = 0; x < width; x++)
        {
            const uint32_t dst_x = (uint32_t)x_start + (uint32_t)x;
            if (dst_x >= epd->width)
            {
                break;
            }

            const uint8_t pixel = get_packed_pixel(image_data, width, x, y, bits_per_pixel);
            epd_set_pixel(epd, (uint16_t)dst_x, (uint16_t)dst_y, pixel, canvas);
        }
    }
}

font_property_t font_properties(text_type_t type)
{
    font_property_t font_property;
    memset(&font_property, 0, sizeof(font_property));
    font_property.type = monospace;

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
        font_property.end = (uint8_t)(FONT_CAMBRIA_16_START_CHAR + FONT_CAMBRIA_16_LENGTH - 1);
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
        font_property.end = (uint8_t)(FONT_TAHOMA_8_START_CHAR + FONT_TAHOMA_8_LENGTH - 1);
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
        font_property.end = (uint8_t)(FONT_TAHOMA_12_START_CHAR + FONT_TAHOMA_12_LENGTH - 1);
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
        font_property.end = (uint8_t)(FONT_TAHOMA_16_START_CHAR + FONT_TAHOMA_16_LENGTH - 1);
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
        font_property.end = (uint8_t)(FONT_TAHOMA_24_START_CHAR + FONT_TAHOMA_24_LENGTH - 1);
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
        font_property.end = (uint8_t)(FONT_NOTO_KUFI_ARABIC_8_START_CHAR + FONT_NOTO_KUFI_ARABIC_8_LENGTH - 1);
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
        font_property.end = (uint8_t)(FONT_NOTO_KUFI_ARABIC_12_START_CHAR + FONT_NOTO_KUFI_ARABIC_12_LENGTH - 1);
        break;
#endif

    default:
        break;
    }

    return font_property;
}
