#include <math.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include <stdlib.h>
#include "epd.h"
#include "canvas.h"
#include "epd_2_13.h"
#include "hardware/adc.h"

#define TEMP_SENSOR_CHANNEL 4

#define TEST_CASE 0

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

    fill_background(&epd, WHITE, canvas);

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
#endif

    epd_display(&epd, canvas);
    epd_sleep(&epd);

    blink(1000);

    return 0;
}