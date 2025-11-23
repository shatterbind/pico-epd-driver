#include <math.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include "epd.h"
#include "canvas.h"
#include "epd_2_13.h"
#include "hardware/adc.h"

#define TEMP_SENSOR_CHANNEL 4

#define TEST_CASE_0_BOXES 0        // colored boxes
#define TEST_CASE_1_TEXT 1         // temperature + text samples
#define TEST_CASE_2_TEXT_BG 2      // text with background
#define TEST_CASE_3_RAYS 3         // sun rays demo
#define TEST_CASE_4_BANDS 4        // horizontal color bands
#define TEST_CASE_5_NESTED_RECTS 5 // nested rectangles demo

#ifndef TEST_CASE
#define TEST_CASE TEST_CASE_5_NESTED_RECTS
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

int main()
{
    static uint8_t canvas[EPD_2_13_WIDTH * EPD_2_13_HEIGHT / 4] = {0};
    epd_t epd = epd_create(8);

    epd_init(&epd);
    stdio_init_all();

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

    draw_text_with_bg(&epd, 3, 198, "asdsadd", FONT_TAHOMA_24, WHITE, 3, BLACK, canvas);
    draw_text_with_bg(&epd, 3, 230, "world", FONT_TAHOMA_24, WHITE, 3, BLACK, canvas);

#elif TEST_CASE == 3
    const color_t palette[] = {BLACK, RED, YELLOW};
    const int step = 16; /* spacing between rays */
    const uint16_t w = epd.width;
    const uint16_t h = epd.height;
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
    const uint16_t w = epd.width;
    const uint16_t h = epd.height;

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

    const uint16_t w = epd.width;
    const uint16_t h = epd.height;

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

            /* cast sizes to expected unsigned types to avoid accidental large values */
            draw_rect(&epd, x, y, (uint16_t)eff_box_w, (uint16_t)eff_box_h, c, canvas);
        }
    }

    // one box with repeated (nested) rectangles inside
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

#endif

    epd_display(&epd, canvas);
    epd_sleep(&epd);

    blink(1000);

    return 0;
}