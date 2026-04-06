#include <math.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include "epd.h"
#include "canvas.h"
#include "epd_2_13.h"
#include "hardware/adc.h"

#define TEMP_SENSOR_CHANNEL 4

#define TEST_CASE_0_BOXES 0         // colored boxes
#define TEST_CASE_1_TEXT 1          // temperature + text samples
#define TEST_CASE_2_TEXT_BG 2       // text with background
#define TEST_CASE_3_RAYS 3          // sun rays demo
#define TEST_CASE_4_BANDS 4         // horizontal color bands
#define TEST_CASE_5_NESTED_RECTS 5  // nested rectangles demo
#define TEST_CASE_6_CIRCLE 6        // circle demo
#define TEST_CASE_7_CIRCLE_FILLED 7 // circle filled demo
#define TEST_CASE_8_IMAGE 8         // draw_image demo

#ifndef TEST_CASE
#define TEST_CASE TEST_CASE_0_BOXES
#endif

float read_onboard_temperature(bool toCelsius)
{
    float adc;
    float tempC;
    const float conversionFactor = 3.3f / (1 << 12);

    adc_select_input(TEMP_SENSOR_CHANNEL);

    adc = (float)adc_read() * conversionFactor;
    tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    return toCelsius ? tempC : tempC * 9 / 5 + 32;
}

static void blink(uint32_t ms)
{
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
    sleep_ms(ms);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

static inline bool is_supported_depth(uint8_t bits_per_pixel)
{
    return bits_per_pixel != 0 && (8 % bits_per_pixel) == 0;
}

static inline void image_set_pixel(uint8_t *image, uint16_t width, uint16_t x, uint16_t y, uint8_t bits_per_pixel, uint8_t color)
{
    if (!image || !is_supported_depth(bits_per_pixel))
    {
        return;
    }

    const uint8_t pixels_per_byte = (uint8_t)(8u / bits_per_pixel);
    const uint32_t index = ((uint32_t)y * (uint32_t)width + (uint32_t)x) / pixels_per_byte;
    const uint8_t shift = (uint8_t)((pixels_per_byte - 1u - (x % pixels_per_byte)) * bits_per_pixel);
    const uint8_t mask = (uint8_t)(((1u << bits_per_pixel) - 1u) << shift);
    image[index] = (uint8_t)((image[index] & ~mask) | ((color & ((1u << bits_per_pixel) - 1u)) << shift));
}

int main()
{
    static uint8_t canvas[EPD_2_13_WIDTH * EPD_2_13_HEIGHT / 4] = {0};
    epd_t epd = epd_create(8);

    epd_init(&epd);
    stdio_init_all();
    epd.rotation = EPD_ROT_270;

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // code
    blink(1000);

#if TEST_CASE == 0
    const int cols = 5;
    const int rows = 10;
    const int box_w = 23;
    const int box_h = 23;
    const int gap = 2;
    const int step_x = box_w + gap;
    const int step_y = box_h + gap;
    const int start_x = 0;
    const int start_y = 0;
    const color_t palette[] = {YELLOW, RED, BLACK};
    const int palette_len = sizeof(palette) / sizeof(palette[0]);

    fill_background(&epd, WHITE, canvas);

    uint32_t rnd = (uint32_t)to_us_since_boot(get_absolute_time());
    if (rnd == 0)
        rnd = 0x12345678u;

    for (int cx = 0; cx < cols; ++cx)
    {
        for (int ry = 0; ry < rows; ++ry)
        {
            rnd ^= rnd << 13;
            rnd ^= rnd >> 17;
            rnd ^= rnd << 5;
            int color = palette[rnd % palette_len];
            int x = start_x + cx * step_x;
            int y = start_y + ry * step_y;
            fill_rect(&epd, x, y, box_w, box_h, color, canvas);
        }
    }
#elif TEST_CASE == 1
    adc_init();
    adc_set_temp_sensor_enabled(true);
    fill_background(&epd, WHITE, canvas);

    char buffer[8] = {0};
    float temperature = read_onboard_temperature(true);

    snprintf(buffer, sizeof(buffer), "%.2f", temperature);

    draw_text(&epd, 0, 0, "Temperature:", FONT_TAHOMA_12, BLACK, 3, canvas);
    draw_text(&epd, 0, 16, buffer, FONT_TAHOMA_12, BLACK, 3, canvas);

    draw_text(&epd, 0, 32, "hello", NOTO_KUFI_ARABIC_8, YELLOW, 3, canvas);
    draw_text(&epd, 0, 42, "world", NOTO_KUFI_ARABIC_8, YELLOW, 3, canvas);

    draw_text(&epd, 0, 55, "hello", FONT_TAHOMA_12, RED, 3, canvas);
    draw_text(&epd, 0, 70, "world", FONT_TAHOMA_12, RED, 3, canvas);

    draw_text(&epd, 0, 84, "hello", FONT_TAHOMA_16, RED, 3, canvas);
    draw_text(&epd, 0, 105, "world", FONT_TAHOMA_16, RED, 3, canvas);

    draw_text(&epd, 0, 125, "hello", FONT_TAHOMA_24, BLACK, 3, canvas);
    draw_text(&epd, 0, 150, "world", FONT_TAHOMA_24, BLACK, 3, canvas);
#elif TEST_CASE == 2
    adc_init();
    adc_set_temp_sensor_enabled(true);

    char buffer[8] = {0};
    float temperature = read_onboard_temperature(true);

    fill_background(&epd, WHITE, canvas);

    snprintf(buffer, sizeof(buffer), "%.2f", temperature);

    draw_text_with_bg(&epd, 3, 0, "Temperature:", FONT_TAHOMA_12, BLACK, 3, YELLOW, canvas);
    draw_text_with_bg(&epd, 3, 16, buffer, FONT_TAHOMA_12, BLACK, 3, YELLOW, canvas);

    draw_text_with_bg(&epd, 3, 32, "hello", NOTO_KUFI_ARABIC_8, YELLOW, 3, BLACK, canvas);
    draw_text_with_bg(&epd, 3, 46, "world", NOTO_KUFI_ARABIC_8, YELLOW, 3, BLACK, canvas);

    draw_text_with_bg(&epd, 3, 60, "hello", FONT_TAHOMA_12, RED, 3, BLACK, canvas);
    draw_text_with_bg(&epd, 3, 76, "world", FONT_TAHOMA_12, RED, 3, BLACK, canvas);

    draw_text_with_bg(&epd, 3, 92, "hello", FONT_TAHOMA_16, BLACK, 3, RED, canvas);
    draw_text_with_bg(&epd, 3, 113, "world", FONT_TAHOMA_16, BLACK, 3, RED, canvas);

    draw_text_with_bg(&epd, 3, 134, "hello", FONT_TAHOMA_24, YELLOW, 3, BLACK, canvas);
    draw_text_with_bg(&epd, 3, 166, "world", FONT_TAHOMA_24, YELLOW, 3, BLACK, canvas);

    draw_text_with_bg(&epd, 3, 198, "hello", FONT_TAHOMA_24, WHITE, 3, BLACK, canvas);
    draw_text_with_bg(&epd, 3, 230, "world", FONT_TAHOMA_24, WHITE, 3, BLACK, canvas);

#elif TEST_CASE == 3
    const color_t palette[] = {BLACK, RED, YELLOW};
    const int step = 16; /* spacing between rays */
    const uint16_t w = canvas_width(&epd);
    const uint16_t h = canvas_height(&epd);
    int idx = 0;

    fill_background(&epd, WHITE, canvas);

    for (int x = step; x < w; x += step)
        draw_line(&epd, 0, 0, x, 0, palette[(idx++) % 3], canvas);

    for (int y = step; y < h; y += step)
        draw_line(&epd, 0, 0, w - 1, y, palette[(idx++) % 3], canvas);

    for (int x = w - 1 - step; x >= 0; x -= step)
        draw_line(&epd, 0, 0, x, h - 1, palette[(idx++) % 3], canvas);

    draw_line(&epd, 0, 0, 0, h, palette[(idx++) % 3], canvas);

#elif TEST_CASE == TEST_CASE_4_BANDS
    const uint16_t w = canvas_width(&epd);
    const uint16_t h = canvas_height(&epd);

    fill_background(&epd, WHITE, canvas);

    for (int y = 0, band = 0; y < h; y += 6, ++band)
    {
        color_t c = (band % 3 == 0) ? BLACK : ((band % 3 == 1) ? RED : YELLOW);
        draw_line(&epd, 0, y, w - 1, y, c, canvas);

        if (y + 1 < h)
        {
            draw_line(&epd, 0, y + 1, w - 1, y + 1, c, canvas);
        }
    }

#elif TEST_CASE == TEST_CASE_5_NESTED_RECTS

    const uint16_t w = canvas_width(&epd);
    const uint16_t h = canvas_height(&epd);

    fill_background(&epd, WHITE, canvas);

    const int cols = 6;
    const int rows = 6;
    const int box_w = 10;
    const int box_h = 10;
    const int gap = 7;
    const int start_x = (w - (cols * box_w + (cols - 1) * gap)) / 2;
    const int start_y = 8;
    const color_t palette[] = {BLACK, RED, YELLOW};

    int eff_box_w = box_w;
    int eff_box_h = box_h;

    int total_w = cols * eff_box_w + (cols - 1) * gap;
    if (total_w > w)
    {
        eff_box_w = (w - (cols - 1) * gap) / cols;
        if (eff_box_w < 1)
            eff_box_w = 1;
    }

    int total_h = rows * eff_box_h + (rows - 1) * gap;
    if (start_y + total_h > h)
    {
        eff_box_h = (h - start_y - (rows - 1) * gap) / rows;
        if (eff_box_h < 1)
            eff_box_h = 1;
    }

    int start_x_adj = (w - (cols * eff_box_w + (cols - 1) * gap)) / 2;
    if (start_x_adj < 0)
        start_x_adj = 0;

    for (int cx = 0; cx < cols; ++cx)
    {
        for (int ry = 0; ry < rows; ++ry)
        {
            color_t c = palette[(cx + ry) % (sizeof(palette) / sizeof(palette[0]))];
            int x = start_x_adj + cx * (eff_box_w + gap);
            int y = start_y + ry * (eff_box_h + gap);

            draw_rect(&epd, x, y, (uint16_t)eff_box_w, (uint16_t)eff_box_h, c, canvas);
        }
    }

    const int outer_w = 120;
    const int outer_h = 120;
    const int outer_x = (w - outer_w) / 2;
    const int outer_y = h - outer_h - 8;
    const int nested_steps = 8;
    const int inset_step = 6; // how much each nested rect is inset

    for (int i = 0; i < nested_steps; ++i)
    {
        int nx = outer_x + i * inset_step;
        int ny = outer_y + i * inset_step;
        int nw = outer_w - i * 2 * inset_step;
        int nh = outer_h - i * 2 * inset_step;
        color_t nc = (i % 2 == 0) ? RED : BLACK;
        draw_rect(&epd, nx, ny, (uint16_t)nw, (uint16_t)nh, nc, canvas);
    }

#elif TEST_CASE == TEST_CASE_6_CIRCLE
    const int cols = 5;
    const int rows = 10;
    const int radius = 11;
    const int gap = 2;
    const int step_x = 2 * radius + gap;
    const int step_y = 2 * radius + gap;
    const int start_x = radius;
    const int start_y = radius;

    const color_t palette[] = {YELLOW, RED, BLACK};
    const int palette_len = sizeof(palette) / sizeof(palette[0]);

    fill_background(&epd, WHITE, canvas);

    uint32_t rnd = (uint32_t)to_us_since_boot(get_absolute_time());

    if (rnd == 0)
        rnd = 0x12345678u;

    for (int cx = 0; cx < cols; ++cx)
    {
        for (int ry = 0; ry < rows; ++ry)
        {
            rnd ^= rnd << 13;
            rnd ^= rnd >> 17;
            rnd ^= rnd << 5;
            int color = palette[rnd % palette_len];
            int x = start_x + cx * step_x;
            int y = start_y + ry * step_y;
            draw_circle(&epd, x, y, radius, color, canvas);
        }
    }

#elif TEST_CASE == TEST_CASE_7_CIRCLE_FILLED
    const int cols = 5;
    const int rows = 10;
    const int radius = 11;
    const int gap = 2;
    const int step_x = 2 * radius + gap;
    const int step_y = 2 * radius + gap;
    const int start_x = radius;
    const int start_y = radius;

    const color_t palette[] = {YELLOW, RED, BLACK};
    const int palette_len = sizeof(palette) / sizeof(palette[0]);

    fill_background(&epd, WHITE, canvas);

    uint32_t rnd = (uint32_t)to_us_since_boot(get_absolute_time());

    if (rnd == 0)
        rnd = 0x12345678u;

    for (int cx = 0; cx < cols; ++cx)
    {
        for (int ry = 0; ry < rows; ++ry)
        {
            rnd ^= rnd << 13;
            rnd ^= rnd >> 17;
            rnd ^= rnd << 5;
            int color = palette[rnd % palette_len];
            int x = start_x + cx * step_x;
            int y = start_y + ry * step_y;

            fill_circle(&epd, x, y, radius, color, canvas);
        }
    }

#elif TEST_CASE == TEST_CASE_8_IMAGE
    const uint8_t bits_per_pixel = (uint8_t)epd.depth;
    if (is_supported_depth(bits_per_pixel))
    {
        enum
        {
            IMG_W = 48,
            IMG_H = 48
        };

        static uint8_t image[IMG_W * IMG_H] = {0}; // allocate for worst case (8bpp)
        memset(image, 0, sizeof(image));
        fill_background(&epd, WHITE, canvas);

        const uint16_t w = canvas_width(&epd);
        const uint16_t h = canvas_height(&epd);
        const uint16_t x0 = (w > IMG_W) ? (uint16_t)((w - IMG_W) / 2) : 0;
        const uint16_t y0 = (h > IMG_H) ? (uint16_t)((h - IMG_H) / 2) : 0;

        for (uint16_t y = 0; y < IMG_H; y++)
        {
            for (uint16_t x = 0; x < IMG_W; x++)
            {
                color_t c = WHITE;

                const bool border = (x == 0) || (y == 0) || (x == IMG_W - 1) || (y == IMG_H - 1);
                if (border)
                {
                    c = BLACK;
                }
                else if (x == y || x + y == IMG_W - 1)
                {
                    c = RED;
                }
                else if (((x / 6) + (y / 6)) % 2 == 0)
                {
                    c = YELLOW;
                }

                image_set_pixel(image, IMG_W, x, y, bits_per_pixel, (uint8_t)c);
            }
        }

        draw_image(&epd, x0, y0, image, IMG_W, IMG_H, canvas);
        draw_rect(&epd, x0, y0, IMG_W, IMG_H, BLACK, canvas);
        draw_text(&epd, 4, 4, "draw_image()", FONT_TAHOMA_12, BLACK, 2, canvas);
    }
#endif

    epd_display(&epd, canvas);
    epd_sleep(&epd);

    blink(1000);

    return 0;
}
