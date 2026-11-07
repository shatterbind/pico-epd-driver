#include <math.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include "epd.h"
#include "canvas.h"
#include "epd_2_13.h"
#include "hardware/adc.h"
#include "images/rpi_logo.h"
#include "images/fullscreen_clean.h"
#include "images/fullscreen_direct.h"
#include "images/fullscreen_dithered.h"
#include "images/watering_template.h"
#include "images/watering_preview.h"

#define TEMP_SENSOR_CHANNEL 4

#define DEMO_DELAY_MS 5000

// Set to 1 to test uploading only the image assets one by one, or 0 to run all demo test cases
#ifndef RUN_IMAGE_TESTS_ONLY
#define RUN_IMAGE_TESTS_ONLY 0
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
    const uint32_t bytes_per_row = ((uint32_t)width * bits_per_pixel + 7u) / 8u;
    const uint32_t index = (uint32_t)y * bytes_per_row + (uint32_t)x / pixels_per_byte;
    const uint8_t shift = (uint8_t)((pixels_per_byte - 1u - (x % pixels_per_byte)) * bits_per_pixel);
    const uint8_t mask = (uint8_t)(((1u << bits_per_pixel) - 1u) << shift);
    image[index] = (uint8_t)((image[index] & ~mask) | ((color & ((1u << bits_per_pixel) - 1u)) << shift));
}


static void run_test_case_0_boxes(epd_t *epd, uint8_t *canvas)
{
    const uint16_t w = canvas_width(epd);
    const uint16_t h = canvas_height(epd);
    const int box_w = 23;
    const int box_h = 23;
    const int gap = 2;
    const int step_x = box_w + gap;
    const int step_y = box_h + gap;
    const int cols = w / step_x;
    const int rows = h / step_y;
    const int start_x = (w - (cols * step_x - gap)) / 2;
    const int start_y = (h - (rows * step_y - gap)) / 2;
    const color_t palette[] = {YELLOW, RED, BLACK};
    const int palette_len = sizeof(palette) / sizeof(palette[0]);

    fill_background(epd, WHITE, canvas);

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
            fill_rect(epd, x, y, box_w, box_h, color, canvas);
        }
    }
}

static void run_test_case_1_text(epd_t *epd, uint8_t *canvas)
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    fill_background(epd, WHITE, canvas);

    char buffer[8] = {0};
    float temperature = read_onboard_temperature(true);

    snprintf(buffer, sizeof(buffer), "%.2f", temperature);

    int x1 = 0;
    int x2 = (canvas_height(epd) < 150) ? canvas_width(epd) / 2 : 0;
    int y2 = (canvas_height(epd) < 150) ? 0 : 125;

    draw_text(epd, x1, 0, "Temperature:", FONT_TAHOMA_12, BLACK, 3, canvas);
    draw_text(epd, x1, 16, buffer, FONT_TAHOMA_12, BLACK, 3, canvas);

    draw_text(epd, x1, 32, "hello", NOTO_KUFI_ARABIC_8, YELLOW, 3, canvas);
    draw_text(epd, x1, 42, "world", NOTO_KUFI_ARABIC_8, YELLOW, 3, canvas);

    draw_text(epd, x1, 55, "hello", FONT_TAHOMA_12, RED, 3, canvas);
    draw_text(epd, x1, 70, "world", FONT_TAHOMA_12, RED, 3, canvas);

    draw_text(epd, x1, 84, "hello", FONT_TAHOMA_16, RED, 3, canvas);
    draw_text(epd, x1, 105, "world", FONT_TAHOMA_16, RED, 3, canvas);

    draw_text(epd, x2, y2, "hello", FONT_TAHOMA_24, BLACK, 3, canvas);
    draw_text(epd, x2, y2 + 25, "world", FONT_TAHOMA_24, BLACK, 3, canvas);
}

static void run_test_case_2_text_bg(epd_t *epd, uint8_t *canvas)
{
    adc_init();
    adc_set_temp_sensor_enabled(true);

    char buffer[8] = {0};
    float temperature = read_onboard_temperature(true);

    fill_background(epd, WHITE, canvas);

    snprintf(buffer, sizeof(buffer), "%.2f", temperature);

    int x1 = 3;
    int x2 = (canvas_height(epd) < 250) ? canvas_width(epd) / 2 : 3;
    int y2 = (canvas_height(epd) < 250) ? 0 : 134;
    int y3 = (canvas_height(epd) < 250) ? y2 + 64 : 198;

    draw_text_with_bg(epd, x1, 0, "Temperature:", FONT_TAHOMA_12, BLACK, 3, YELLOW, canvas);
    draw_text_with_bg(epd, x1, 16, buffer, FONT_TAHOMA_12, BLACK, 3, YELLOW, canvas);

    draw_text_with_bg(epd, x1, 32, "hello", NOTO_KUFI_ARABIC_8, YELLOW, 3, BLACK, canvas);
    draw_text_with_bg(epd, x1, 46, "world", NOTO_KUFI_ARABIC_8, YELLOW, 3, BLACK, canvas);

    draw_text_with_bg(epd, x1, 60, "hello", FONT_TAHOMA_12, RED, 3, BLACK, canvas);
    draw_text_with_bg(epd, x1, 76, "world", FONT_TAHOMA_12, RED, 3, BLACK, canvas);

    draw_text_with_bg(epd, x1, 92, "hello", FONT_TAHOMA_16, BLACK, 3, RED, canvas);
    draw_text_with_bg(epd, x1, 113, "world", FONT_TAHOMA_16, BLACK, 3, RED, canvas);

    draw_text_with_bg(epd, x2, y2, "hello", FONT_TAHOMA_24, YELLOW, 3, BLACK, canvas);
    draw_text_with_bg(epd, x2, y2 + 32, "world", FONT_TAHOMA_24, YELLOW, 3, BLACK, canvas);

    draw_text_with_bg(epd, x2, y3, "hello", FONT_TAHOMA_24, WHITE, 3, BLACK, canvas);
    draw_text_with_bg(epd, x2, y3 + 32, "world", FONT_TAHOMA_24, WHITE, 3, BLACK, canvas);
}

static void run_test_case_3_rays(epd_t *epd, uint8_t *canvas)
{
    const color_t palette[] = {BLACK, RED, YELLOW};
    const int step = 16; /* spacing between rays */
    const uint16_t w = canvas_width(epd);
    const uint16_t h = canvas_height(epd);
    int idx = 0;

    fill_background(epd, WHITE, canvas);

    for (int x = step; x < w; x += step)
        draw_line(epd, 0, 0, x, 0, palette[(idx++) % 3], canvas);

    for (int y = step; y < h; y += step)
        draw_line(epd, 0, 0, w - 1, y, palette[(idx++) % 3], canvas);

    for (int x = w - 1 - step; x >= 0; x -= step)
        draw_line(epd, 0, 0, x, h - 1, palette[(idx++) % 3], canvas);

    draw_line(epd, 0, 0, 0, h, palette[(idx++) % 3], canvas);
}

static void run_test_case_4_bands(epd_t *epd, uint8_t *canvas)
{
    const uint16_t w = canvas_width(epd);
    const uint16_t h = canvas_height(epd);

    fill_background(epd, WHITE, canvas);

    for (int y = 0, band = 0; y < h; y += 6, ++band)
    {
        color_t c = (band % 3 == 0) ? BLACK : ((band % 3 == 1) ? RED : YELLOW);
        draw_line(epd, 0, y, w - 1, y, c, canvas);

        if (y + 1 < h)
        {
            draw_line(epd, 0, y + 1, w - 1, y + 1, c, canvas);
        }
    }
}

static void run_test_case_5_nested_rects(epd_t *epd, uint8_t *canvas)
{
    const uint16_t w = canvas_width(epd);
    const uint16_t h = canvas_height(epd);

    fill_background(epd, WHITE, canvas);

    const int cols = 6;
    const int rows = 6;
    const int box_w = 10;
    const int box_h = 10;
    const int gap = 7;
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

            draw_rect(epd, x, y, (uint16_t)eff_box_w, (uint16_t)eff_box_h, c, canvas);
        }
    }

    int outer_w = 120;
    int outer_h = 120;
    if (outer_w > w) outer_w = w - 8;
    if (outer_h > h) outer_h = h - 8;
    if (outer_w > outer_h) outer_w = outer_h; else outer_h = outer_w;
    
    int outer_x = (w - outer_w) / 2;
    int outer_y = h - outer_h - 8;
    if (outer_y < 0) outer_y = 0;
    const int nested_steps = 8;
    const int inset_step = outer_w / (nested_steps * 2 + 2); // adapt inset_step

    for (int i = 0; i < nested_steps; ++i)
    {
        int nx = outer_x + i * inset_step;
        int ny = outer_y + i * inset_step;
        int nw = outer_w - i * 2 * inset_step;
        int nh = outer_h - i * 2 * inset_step;
        color_t nc = (i % 2 == 0) ? RED : BLACK;
        draw_rect(epd, nx, ny, (uint16_t)nw, (uint16_t)nh, nc, canvas);
    }
}

static void run_test_case_6_circle(epd_t *epd, uint8_t *canvas)
{
    const uint16_t w = canvas_width(epd);
    const uint16_t h = canvas_height(epd);
    const int radius = 11;
    const int gap = 2;
    const int step_x = 2 * radius + gap;
    const int step_y = 2 * radius + gap;
    const int cols = w / step_x;
    const int rows = h / step_y;
    const int start_x = (w - (cols * step_x - gap)) / 2 + radius;
    const int start_y = (h - (rows * step_y - gap)) / 2 + radius;

    const color_t palette[] = {YELLOW, RED, BLACK};
    const int palette_len = sizeof(palette) / sizeof(palette[0]);

    fill_background(epd, WHITE, canvas);

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
            draw_circle(epd, x, y, radius, color, canvas);
        }
    }
}

static void run_test_case_7_circle_filled(epd_t *epd, uint8_t *canvas)
{
    const uint16_t w = canvas_width(epd);
    const uint16_t h = canvas_height(epd);
    const int radius = 11;
    const int gap = 2;
    const int step_x = 2 * radius + gap;
    const int step_y = 2 * radius + gap;
    const int cols = w / step_x;
    const int rows = h / step_y;
    const int start_x = (w - (cols * step_x - gap)) / 2 + radius;
    const int start_y = (h - (rows * step_y - gap)) / 2 + radius;

    const color_t palette[] = {YELLOW, RED, BLACK};
    const int palette_len = sizeof(palette) / sizeof(palette[0]);

    fill_background(epd, WHITE, canvas);

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

            fill_circle(epd, x, y, radius, color, canvas);
        }
    }
}

static void run_test_case_8_image(epd_t *epd, uint8_t *canvas)
{
    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    if (is_supported_depth(bits_per_pixel))
    {
        fill_background(epd, WHITE, canvas);

        const uint16_t w = canvas_width(epd);
        const uint16_t h = canvas_height(epd);
        const uint16_t x0 = (w > RPI_LOGO_WIDTH) ? (uint16_t)((w - RPI_LOGO_WIDTH) / 2) : 0;
        const uint16_t y0 = (h > RPI_LOGO_HEIGHT) ? (uint16_t)((h - RPI_LOGO_HEIGHT) / 2) : 0;

        if (bits_per_pixel == 2)
        {
            draw_image(epd, x0, y0, rpi_logo_64x64, RPI_LOGO_WIDTH, RPI_LOGO_HEIGHT, canvas);
        }
        else
        {
            static uint8_t converted_image[(RPI_LOGO_WIDTH * RPI_LOGO_HEIGHT + 7) / 8 * 8] = {0};
            memset(converted_image, 0, sizeof(converted_image));
            for (uint16_t y = 0; y < RPI_LOGO_HEIGHT; y++)
            {
                for (uint16_t x = 0; x < RPI_LOGO_WIDTH; x++)
                {
                    const uint32_t byte_idx = (uint32_t)y * ((RPI_LOGO_WIDTH * 2 + 7) / 8) + (uint32_t)x / 4;
                    const uint8_t shift = (uint8_t)((3 - (x % 4)) * 2);
                    const uint8_t color = (uint8_t)((rpi_logo_64x64[byte_idx] >> shift) & 0x03);
                    image_set_pixel(converted_image, RPI_LOGO_WIDTH, x, y, bits_per_pixel, color);
                }
            }
            draw_image(epd, x0, y0, converted_image, RPI_LOGO_WIDTH, RPI_LOGO_HEIGHT, canvas);
        }

        draw_rect(epd, x0, y0, RPI_LOGO_WIDTH, RPI_LOGO_HEIGHT, BLACK, canvas);

        const char *title = "draw_image()";
        uint16_t title_w = canvas_get_text_width(title, FONT_TAHOMA_12, 2);
        uint16_t title_x = (w > title_w) ? (w - title_w) / 2 : 4;
        draw_text(epd, title_x, 4, title, FONT_TAHOMA_12, BLACK, 2, canvas);
    }
}

static void draw_fullscreen_image(epd_t *epd, const uint8_t *image_data, uint16_t width, uint16_t height, uint8_t *canvas)
{
    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    if (!is_supported_depth(bits_per_pixel))
    {
        return;
    }

    fill_background(epd, WHITE, canvas);

    const uint16_t w = canvas_width(epd);
    const uint16_t h = canvas_height(epd);
    const uint16_t x0 = (w > width) ? (uint16_t)((w - width) / 2) : 0;
    const uint16_t y0 = (h > height) ? (uint16_t)((h - height) / 2) : 0;

    if (bits_per_pixel == 2)
    {
        draw_image(epd, x0, y0, image_data, width, height, canvas);
    }
    else
    {
        for (uint16_t y = 0; y < height && (y0 + y) < h; y++)
        {
            for (uint16_t x = 0; x < width && (x0 + x) < w; x++)
            {
                const uint32_t byte_idx = (uint32_t)y * ((width * 2 + 7) / 8) + (uint32_t)x / 4;
                const uint8_t shift = (uint8_t)((3 - (x % 4)) * 2);
                const uint8_t color = (uint8_t)((image_data[byte_idx] >> shift) & 0x03);
                epd_set_pixel(epd, x0 + x, y0 + y, color, canvas);
            }
        }
    }
}

static void run_test_case_image_clean(epd_t *epd, uint8_t *canvas)
{
    draw_fullscreen_image(epd, fullscreen_clean_122x250, FULLSCREEN_CLEAN_WIDTH, FULLSCREEN_CLEAN_HEIGHT, canvas);
}

static void run_test_case_image_direct(epd_t *epd, uint8_t *canvas)
{
    draw_fullscreen_image(epd, fullscreen_direct_122x250, FULLSCREEN_DIRECT_WIDTH, FULLSCREEN_DIRECT_HEIGHT, canvas);
}

static void run_test_case_image_dithered(epd_t *epd, uint8_t *canvas)
{
    draw_fullscreen_image(epd, fullscreen_dithered_122x250, FULLSCREEN_DITHERED_WIDTH, FULLSCREEN_DITHERED_HEIGHT, canvas);
}

static void run_test_case_watering_dashboard(epd_t *epd, uint8_t *canvas)
{
    // Draw the static template image as background
    draw_fullscreen_image(epd, watering_template_122x250, WATERING_TEMPLATE_WIDTH, WATERING_TEMPLATE_HEIGHT, canvas);

    // Overlay dynamic live data into designated template slots:
    // 1. Battery voltage (with white background clearing the template slot)
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2fV", 3.85f);
    draw_text_with_bg(epd, 86, 4, buf, FONT_TAHOMA_8, BLACK, 1, WHITE, canvas);

    // 2. Plant name
    draw_text_with_bg(epd, 12, 38, "MONSTERA", FONT_TAHOMA_12, BLACK, 1, WHITE, canvas);
    draw_text_with_bg(epd, 12, 56, "DELICIOSA", FONT_TAHOMA_8, RED, 1, WHITE, canvas);

    // 3. Soil moisture / humidity
    const int moisture_pct = 68;
    snprintf(buf, sizeof(buf), "%d%%", moisture_pct);
    draw_text_with_bg(epd, 42, 94, buf, FONT_TAHOMA_16, BLACK, 1, WHITE, canvas);
    draw_text_with_bg(epd, 42, 112, "OPTIMAL", FONT_TAHOMA_8, RED, 1, WHITE, canvas);

    // Fill the progress gauge bar: 95 pixels total width (13 to 108)
    const uint16_t fill_w = (uint16_t)(95 * moisture_pct / 100);
    if (fill_w > 0)
    {
        fill_rect(epd, 13, 127, fill_w, 6, YELLOW, canvas);
    }

    // 4. Temperature
    float temp_c = read_onboard_temperature(true);
    if (temp_c < 10.0f || temp_c > 45.0f)
    {
        temp_c = 24.5f; // Fallback demo default
    }
    snprintf(buf, sizeof(buf), "%.1f C", temp_c);
    draw_text_with_bg(epd, 42, 166, buf, FONT_TAHOMA_16, BLACK, 1, WHITE, canvas);

    // 5. Last refresh date-time
    draw_text_with_bg(epd, 30, 224, "11/07 08:30", FONT_TAHOMA_8, RED, 1, WHITE, canvas);
}

static void run_test_case_9_shapes(epd_t *epd, uint8_t *canvas)
{
    const uint16_t w = canvas_width(epd);
    const uint16_t h = canvas_height(epd);

    fill_background(epd, WHITE, canvas);

    const char *title = "Primitives Demo";
    uint16_t title_w = canvas_get_text_width(title, FONT_TAHOMA_12, 2);
    uint16_t title_x = (w > title_w) ? (w - title_w) / 2 : 2;
    draw_text(epd, title_x, 4, title, FONT_TAHOMA_12, BLACK, 2, canvas);

    // Rounded rectangles
    draw_round_rect(epd, 6, 24, (w > 20) ? (w - 12) : 20, 36, 8, RED, canvas);
    fill_round_rect(epd, 12, 30, (w > 32) ? (w - 24) : 10, 24, 5, YELLOW, canvas);

    // Triangles
    uint16_t mid_x = w / 2;
    uint16_t y_base = (h > 60) ? (h - 15) : (h - 5);
    uint16_t y_top = (y_base > 40) ? (y_base - 40) : 20;

    draw_triangle(epd, mid_x - 30, y_base, mid_x, y_top, mid_x + 30, y_base, BLACK, canvas);
    fill_triangle(epd, mid_x - 15, y_base - 5, mid_x, y_top + 10, mid_x + 15, y_base - 5, RED, canvas);
}

typedef void (*test_func_t)(epd_t *, uint8_t *);

int main()
{
    static uint8_t canvas[((EPD_2_13_WIDTH * 2 + 7) / 8) * EPD_2_13_HEIGHT] = {0};
    epd_t epd = epd_create(8);

    epd_init(&epd);
    stdio_init_all();

    adc_init();
    adc_set_temp_sensor_enabled(true);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    blink(1000);

    const epd_rotation_t rotations[] = {EPD_ROT_0, EPD_ROT_90, EPD_ROT_180, EPD_ROT_270};
    const int num_rotations = sizeof(rotations) / sizeof(rotations[0]);

#if RUN_IMAGE_TESTS_ONLY
    const test_func_t tests[] = {
        run_test_case_watering_dashboard,
        run_test_case_8_image,
        run_test_case_image_clean,
        run_test_case_image_direct,
        run_test_case_image_dithered,
    };
#else
    const test_func_t tests[] = {
        run_test_case_watering_dashboard,
        run_test_case_0_boxes,
        run_test_case_1_text,
        run_test_case_2_text_bg,
        run_test_case_3_rays,
        run_test_case_4_bands,
        run_test_case_5_nested_rects,
        run_test_case_6_circle,
        run_test_case_7_circle_filled,
        run_test_case_8_image,
        run_test_case_image_clean,
        run_test_case_image_direct,
        run_test_case_image_dithered,
        run_test_case_9_shapes,
    };
#endif

    const int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int rot = 0; rot < num_rotations; rot++)
    {
        epd.rotation = rotations[rot];

        for (int t = 0; t < num_tests; t++)
        {
            tests[t](&epd, canvas);
            epd_display(&epd, canvas);
            sleep_ms(DEMO_DELAY_MS);
        }
    }

    epd_sleep(&epd);
    blink(1000);

    return 0;
}
