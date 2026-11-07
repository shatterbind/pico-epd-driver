#include "canvas.h"
#include <stdbool.h>
#include <string.h>

#ifdef USE_NOTO_KUFI_ARABIC_8
#include "fonts/noto_kufi_arabic_8_font.h"
#endif

#ifdef USE_NOTO_KUFI_ARABIC_12
#include "fonts/noto_kufi_arabic_12_font.h"
#endif

#ifdef USE_FONT_CAMBRIA_16
#include "fonts/font_cambria_16.h"
#endif

#ifdef USE_FONT_TAHOMA_8
#include "fonts/tahoma_8_font.h"
#endif

#ifdef USE_FONT_TAHOMA_12
#include "fonts/tahoma_12_font.h"
#endif

#ifdef USE_FONT_TAHOMA_16
#include "fonts/tahoma_16_font.h"
#endif

#ifdef USE_FONT_TAHOMA_24
#include "fonts/tahoma_24_font.h"
#endif


static inline bool is_supported_depth(uint8_t bits_per_pixel)
{
    return bits_per_pixel != 0 && (8 % bits_per_pixel) == 0;
}

uint16_t canvas_width(const epd_t *epd)
{
    if (!epd)
    {
        return 0;
    }

    switch (epd->rotation)
    {
    case EPD_ROT_90:
    case EPD_ROT_270: return epd->height;
    case EPD_ROT_0:
    case EPD_ROT_180:
    default: return epd->width;
    }
}

uint16_t canvas_height(const epd_t *epd)
{
    if (!epd)
    {
        return 0;
    }

    switch (epd->rotation)
    {
    case EPD_ROT_90:
    case EPD_ROT_270:
        return epd->width;
    case EPD_ROT_0:
    case EPD_ROT_180:
    default:
        return epd->height;
    }
}

static inline bool is_not_printable(uint8_t c, font_property_t font_property)
{
    return c < font_property.start || c > font_property.end;
}

static inline bool map_logical_to_physical(const epd_t *epd, uint16_t x, uint16_t y, uint16_t *out_x, uint16_t *out_y)
{
    if (!epd || !out_x || !out_y)
    {
        return false;
    }

    const uint16_t w = epd->width;
    const uint16_t h = epd->height;

    switch (epd->rotation)
    {
    case EPD_ROT_0:
    default:
        *out_x = x;
        *out_y = y;
        return true;

    case EPD_ROT_180:
        *out_x = (uint16_t)(w - 1u - x);
        *out_y = (uint16_t)(h - 1u - y);
        return true;

    case EPD_ROT_90:
        *out_x = y;
        *out_y = (uint16_t)(h - 1u - x);
        return true;

    case EPD_ROT_270:
        *out_x = (uint16_t)(w - 1u - y);
        *out_y = x;
        return true;
    }
}

void epd_set_pixel(epd_t *epd, uint16_t x, uint16_t y, uint8_t color, uint8_t *canvas)
{
    if (!epd || !canvas)
    {
        return;
    }

    const uint16_t logical_w = canvas_width(epd);
    const uint16_t logical_h = canvas_height(epd);
    if (x >= logical_w || y >= logical_h)
    {
        return;
    }

    uint16_t px = 0;
    uint16_t py = 0;
    if (!map_logical_to_physical(epd, x, y, &px, &py))
    {
        return;
    }

    if (px >= epd->width || py >= epd->height)
    {
        return;
    }

    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    if (!is_supported_depth(bits_per_pixel))
    {
        return;
    }

    const uint8_t pixels_per_byte = 8 / bits_per_pixel;
    const uint32_t bytes_per_row = ((uint32_t)epd->width * bits_per_pixel + 7u) / 8u;
    const uint32_t index = (uint32_t)py * bytes_per_row + (uint32_t)px / pixels_per_byte;
    uint8_t shift = (pixels_per_byte - 1 - (px % pixels_per_byte)) * bits_per_pixel;

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

    const uint8_t pixels_per_byte = (uint8_t)(CHAR_BIT / bits_per_pixel);
    const uint32_t bytes_per_row = ((uint32_t)epd->width * bits_per_pixel + 7u) / 8u;
    const uint32_t total_bytes = bytes_per_row * epd->height;

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

uint16_t canvas_get_text_width(const char *text, text_type_t type, uint8_t gap)
{
    if (!text || *text == '\0')
    {
        return 0;
    }

    const font_property_t font = font_properties(type);
    if (!font.pointer || font.width == 0 || font.height == 0 || font.length == 0)
    {
        return 0;
    }

    uint16_t total_width = 0;
    bool first_char = true;

    for (const char *p = text; *p != '\0'; ++p)
    {
        const uint8_t c = (uint8_t)*p;
        if (is_not_printable(c, font))
        {
            continue;
        }

        uint16_t char_w = font.width;
        if (font.type == proportional)
        {
            uint32_t offset = (uint32_t)(c - font.start) * (uint32_t)font.length;
            char_w = font.pointer[offset];
        }

        if (!first_char)
        {
            total_width += gap;
        }
        total_width += char_w;
        first_char = false;
    }

    return total_width;
}

uint16_t canvas_get_text_height(text_type_t type)
{
    const font_property_t font = font_properties(type);
    return font.height;
}

void draw_line(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas)
    {
        return;
    }

    int32_t x_start = x0;
    int32_t y_start = y0;
    int32_t x_end = x1;
    int32_t y_end = y1;

    int32_t dx = abs((int)(x_end - x_start));
    int32_t dy = abs((int)(y_end - y_start));

    int32_t sx = (x_start < x_end) ? 1 : -1;
    int32_t sy = (y_start < y_end) ? 1 : -1;

    int32_t err = dx - dy;
    int32_t e2;

    while (true)
    {
        epd_set_pixel(epd, (uint16_t)x_start, (uint16_t)y_start, color, canvas);

        if (x_start == x_end && y_start == y_end)
        {
            break;
        }

        e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x_start += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y_start += sy;
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

    const uint16_t logical_w = canvas_width(epd);
    const uint16_t logical_h = canvas_height(epd);

    uint32_t x_end_coord = (uint32_t)x + (uint32_t)width;
    uint32_t y_end_coord = (uint32_t)y + (uint32_t)height;

    if (x_end_coord > logical_w)
    {
        x_end_coord = logical_w;
    }
    if (y_end_coord > logical_h)
    {
        y_end_coord = logical_h;
    }

    if (x >= x_end_coord || y >= y_end_coord)
    {
        return;
    }

    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    if (!is_supported_depth(bits_per_pixel))
    {
        return;
    }

    // Determine physical rectangle coordinates based on rotation
    uint16_t px_start, px_end, py_start, py_end;
    const uint16_t w = epd->width;
    const uint16_t h = epd->height;

    switch (epd->rotation)
    {
    case EPD_ROT_0:
    default:
        px_start = x;
        px_end = (uint16_t)x_end_coord;
        py_start = y;
        py_end = (uint16_t)y_end_coord;
        break;

    case EPD_ROT_180:
        px_start = w - (uint16_t)x_end_coord;
        px_end = w - x;
        py_start = h - (uint16_t)y_end_coord;
        py_end = h - y;
        break;

    case EPD_ROT_90:
        px_start = y;
        px_end = (uint16_t)y_end_coord;
        py_start = h - (uint16_t)x_end_coord;
        py_end = h - x;
        break;

    case EPD_ROT_270:
        px_start = w - (uint16_t)y_end_coord;
        px_end = w - y;
        py_start = x;
        py_end = (uint16_t)x_end_coord;
        break;
    }

    const uint8_t pixels_per_byte = 8 / bits_per_pixel;
    const uint8_t pixel_mask = (1 << bits_per_pixel) - 1;
    const uint32_t bytes_per_row = ((uint32_t)w * bits_per_pixel + 7u) / 8u;

    uint8_t packed_color = 0;
    for (uint8_t i = 0; i < pixels_per_byte; i++)
    {
        packed_color |= (uint8_t)((color & pixel_mask) << (i * bits_per_pixel));
    }

    const uint16_t byte_start = px_start / pixels_per_byte;
    const uint16_t byte_end = px_end / pixels_per_byte;

    for (uint16_t py = py_start; py < py_end; py++)
    {
        uint8_t *row_ptr = canvas + (uint32_t)py * bytes_per_row;

        if (byte_start == byte_end)
        {
            uint8_t mask = 0;
            uint8_t val = 0;
            for (uint16_t px = px_start; px < px_end; px++)
            {
                uint8_t shift = (pixels_per_byte - 1 - (px % pixels_per_byte)) * bits_per_pixel;
                mask |= (uint8_t)(pixel_mask << shift);
                val |= (uint8_t)((color & pixel_mask) << shift);
            }
            row_ptr[byte_start] = (row_ptr[byte_start] & ~mask) | val;
        }
        else
        {
            // Unaligned start
            {
                uint8_t mask = 0;
                uint8_t val = 0;
                const uint16_t align_end = (byte_start + 1) * pixels_per_byte;
                for (uint16_t px = px_start; px < align_end; px++)
                {
                    uint8_t shift = (pixels_per_byte - 1 - (px % pixels_per_byte)) * bits_per_pixel;
                    mask |= (uint8_t)(pixel_mask << shift);
                    val |= (uint8_t)((color & pixel_mask) << shift);
                }
                row_ptr[byte_start] = (row_ptr[byte_start] & ~mask) | val;
            }

            // Aligned middle
            if (byte_start + 1 < byte_end)
            {
                memset(row_ptr + byte_start + 1, packed_color, byte_end - (byte_start + 1));
            }

            // Unaligned end
            if (px_end > byte_end * pixels_per_byte)
            {
                uint8_t mask = 0;
                uint8_t val = 0;
                for (uint16_t px = byte_end * pixels_per_byte; px < px_end; px++)
                {
                    uint8_t shift = (pixels_per_byte - 1 - (px % pixels_per_byte)) * bits_per_pixel;
                    mask |= (uint8_t)(pixel_mask << shift);
                    val |= (uint8_t)((color & pixel_mask) << shift);
                }
                row_ptr[byte_end] = (row_ptr[byte_end] & ~mask) | val;
            }
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

static void draw_circle_helper(epd_t *epd, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, color_t color, uint8_t *canvas)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x4)
        {
            epd_set_pixel(epd, x0 + x, y0 + y, color, canvas);
            epd_set_pixel(epd, x0 + y, y0 + x, color, canvas);
        }
        if (cornername & 0x2)
        {
            epd_set_pixel(epd, x0 + x, y0 - y, color, canvas);
            epd_set_pixel(epd, x0 + y, y0 - x, color, canvas);
        }
        if (cornername & 0x8)
        {
            epd_set_pixel(epd, x0 - y, y0 + x, color, canvas);
            epd_set_pixel(epd, x0 - x, y0 + y, color, canvas);
        }
        if (cornername & 0x1)
        {
            epd_set_pixel(epd, x0 - y, y0 - x, color, canvas);
            epd_set_pixel(epd, x0 - x, y0 - y, color, canvas);
        }
    }
}

static void fill_circle_helper(epd_t *epd, int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, color_t color, uint8_t *canvas)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    int16_t px = x;
    int16_t py = y;

    delta++;

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (x < (y + 1))
        {
            if (corners & 1)
            {
                draw_line(epd, x0 - y, y0 - x, x0 - y, y0 - x + 2 * x + delta - 1, color, canvas);
            }
            if (corners & 2)
            {
                draw_line(epd, x0 + y, y0 - x, x0 + y, y0 - x + 2 * x + delta - 1, color, canvas);
            }
        }
        if (y != py)
        {
            if (corners & 1)
            {
                draw_line(epd, x0 - px, y0 - py, x0 - px, y0 - py + 2 * py + delta - 1, color, canvas);
            }
            if (corners & 2)
            {
                draw_line(epd, x0 + px, y0 - py, x0 + px, y0 - py + 2 * py + delta - 1, color, canvas);
            }
            py = y;
        }
        px = x;
    }
}

void draw_round_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas || width == 0 || height == 0)
    {
        return;
    }

    uint16_t max_r = ((width < height) ? width : height) / 2;
    if (radius > max_r)
    {
        radius = max_r;
    }

    if (radius == 0)
    {
        draw_rect(epd, x, y, width, height, color, canvas);
        return;
    }

    draw_line(epd, x + radius, y, x + width - radius - 1, y, color, canvas);
    draw_line(epd, x + radius, y + height - 1, x + width - radius - 1, y + height - 1, color, canvas);
    draw_line(epd, x, y + radius, x, y + height - radius - 1, color, canvas);
    draw_line(epd, x + width - 1, y + radius, x + width - 1, y + height - radius - 1, color, canvas);

    draw_circle_helper(epd, x + radius, y + radius, radius, 1, color, canvas);
    draw_circle_helper(epd, x + width - radius - 1, y + radius, radius, 2, color, canvas);
    draw_circle_helper(epd, x + width - radius - 1, y + height - radius - 1, radius, 4, color, canvas);
    draw_circle_helper(epd, x + radius, y + height - radius - 1, radius, 8, color, canvas);
}

void fill_round_rect(epd_t *epd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t radius, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas || width == 0 || height == 0)
    {
        return;
    }

    uint16_t max_r = ((width < height) ? width : height) / 2;
    if (radius > max_r)
    {
        radius = max_r;
    }

    if (radius == 0)
    {
        fill_rect(epd, x, y, width, height, color, canvas);
        return;
    }

    fill_rect(epd, x + radius, y, width - 2 * radius, height, color, canvas);
    fill_circle_helper(epd, x + width - radius - 1, y + radius, radius, 1, height - 2 * radius - 1, color, canvas);
    fill_circle_helper(epd, x + radius, y + radius, radius, 2, height - 2 * radius - 1, color, canvas);
}

void draw_triangle(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint8_t *canvas)
{
    draw_line(epd, x0, y0, x1, y1, color, canvas);
    draw_line(epd, x1, y1, x2, y2, color, canvas);
    draw_line(epd, x2, y2, x0, y0, color, canvas);
}

void fill_triangle(epd_t *epd, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, color_t color, uint8_t *canvas)
{
    if (!epd || !canvas)
    {
        return;
    }

    int16_t a, b, y, last;

    if (y0 > y1) {
        int16_t t;
        t = y0; y0 = y1; y1 = t;
        t = x0; x0 = x1; x1 = t;
    }
    if (y1 > y2) {
        int16_t t;
        t = y2; y2 = y1; y1 = t;
        t = x2; x2 = x1; x1 = t;
    }
    if (y0 > y1) {
        int16_t t;
        t = y0; y0 = y1; y1 = t;
        t = x0; x0 = x1; x1 = t;
    }

    if (y0 == y2)
    {
        a = b = x0;
        if (x1 < a) a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a) a = x2;
        else if (x2 > b) b = x2;
        draw_line(epd, a, y0, b, y0, color, canvas);
        return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0;
    int16_t dx02 = x2 - x0, dy02 = y2 - y0;
    int16_t dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    if (y1 == y2) last = y1;
    else last = y1 - 1;

    for (y = y0; y <= last; y++)
    {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        if (a > b) { int16_t t = a; a = b; b = t; }
        draw_line(epd, a, y, b, y, color, canvas);
    }

    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++)
    {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b) { int16_t t = a; a = b; b = t; }
        draw_line(epd, a, y, b, y, color, canvas);
    }
}


static inline uint8_t get_packed_pixel(const uint8_t *buffer, uint16_t width, uint16_t x, uint16_t y, uint8_t bits_per_pixel)
{
    const uint8_t pixels_per_byte = (uint8_t)(8u / bits_per_pixel);
    const uint32_t bytes_per_row = ((uint32_t)width * bits_per_pixel + 7u) / 8u;
    const uint32_t index = (uint32_t)y * bytes_per_row + (uint32_t)x / pixels_per_byte;
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

    const uint16_t logical_w = canvas_width(epd);
    const uint16_t logical_h = canvas_height(epd);

    for (uint16_t y = 0; y < height; y++)
    {
        const uint32_t dst_y = (uint32_t)y_start + (uint32_t)y;
        if (dst_y >= logical_h)
        {
            break;
        }

        for (uint16_t x = 0; x < width; x++)
        {
            const uint32_t dst_x = (uint32_t)x_start + (uint32_t)x;
            if (dst_x >= logical_w)
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
