#include "epd_2_13.h"
#include "epd.h"
#include <string.h>

static const uint8_t epd_2_13_init_cmds[] = {
    2, 0x00, 0x0f, 0x29,
    2, 0x01, 0x07, 0x00,
    3, 0x03, 0x10, 0x54, 0x44,
    7, 0x06, 0x05, 0x00, 0x3f, 0x0a, 0x25, 0x12, 0x1a,
    1, 0x50, 0x37,
    2, 0x60, 0x02, 0x02,
    4, 0x61, 0x00, 0x7C, 0x00, 0xfa,
    1, 0xe3, 0xFF,
    1, 0xe7, 0x1c,
    1, 0xe9, 0x01,
    3, 0xb4, 0xd0, 0xb5, 0x03,
    1, 0x30, 0x08,
    0, 0x04};

epd_t epd_create_custom(spi_inst_t *spi, pin_cfg_t pin_cfg, uint16_t width, uint16_t height, depth_t depth, uint32_t baud_rate)
{
    epd_t epd;

    epd.width = width;
    epd.height = height;
    epd.baud_rate = baud_rate;
    epd.depth = depth;
    epd.rotation = EPD_ROT_0;
    epd.spi_port = (spi != NULL) ? spi : spi1;
    epd.pin_cfg = pin_cfg;
    epd.gpio_mask = (1u << pin_cfg.dc) | (1u << pin_cfg.cs);

    return epd;
}

epd_t epd_create(uint8_t dc)
{
    pin_cfg_t pin_cfg;

    pin_cfg.cs = EPD_2_13_PIN_CS;
    pin_cfg.dc = dc;
    pin_cfg.mosi = EPD_2_13_PIN_MOSI;
    pin_cfg.rst = EPD_2_13_PIN_RESET;
    pin_cfg.sck = EPD_2_13_PIN_CLK;
    pin_cfg.busy = EPD_2_13_PIN_BUSY;

    return epd_create_custom(spi1, pin_cfg, EPD_2_13_WIDTH, EPD_2_13_HEIGHT, DEPTH_2BIT, BAUD_RATE);
}

void epd_init(epd_t *epd)
{
    spi_init(epd->spi_port, epd->baud_rate);

    gpio_set_function(epd->pin_cfg.sck, GPIO_FUNC_SPI);
    gpio_set_function(epd->pin_cfg.mosi, GPIO_FUNC_SPI);

    gpio_init(epd->pin_cfg.cs);
    gpio_init(epd->pin_cfg.dc);
    gpio_init(epd->pin_cfg.rst);
    gpio_init(epd->pin_cfg.busy);

    gpio_set_dir(epd->pin_cfg.cs, GPIO_OUT);
    gpio_set_dir(epd->pin_cfg.dc, GPIO_OUT);
    gpio_set_dir(epd->pin_cfg.rst, GPIO_OUT);
    gpio_set_dir(epd->pin_cfg.busy, GPIO_IN);

    epd_set_dc_cs(epd, 0, 1);

    epd_reset(epd);

    uint16_t offset_ptr = 0;
    uint8_t command = 0;
    uint8_t length_data = 0;

    epd_read_busy(epd);

    while (offset_ptr < sizeof(epd_2_13_init_cmds))
    {
        command = *(epd_2_13_init_cmds + offset_ptr + OFFSET_CMD);
        length_data = *(epd_2_13_init_cmds + offset_ptr);

        epd_send_command(epd, command);

        if (0 != length_data)
        {
            epd_send_data(epd, epd_2_13_init_cmds + offset_ptr + OFFSET_DATA, length_data);
        }

        offset_ptr += 2 + length_data;
    }

    epd_read_busy(epd);
}

void epd_turn_on(epd_t *epd)
{
    const uint8_t cmd = 0x12;
    const uint8_t data = 0x00;

    epd_send_command(epd, cmd);
    epd_send_data(epd, &data, sizeof(uint8_t));
    epd_read_busy(epd);
}

void epd_send_command(epd_t *epd, uint8_t command)
{
    epd_set_dc_cs(epd, 0, 0);
    spi_write_blocking(epd->spi_port, &command, 1);
    epd_set_dc_cs(epd, 0, 1);
}

void epd_send_data(epd_t *epd, const uint8_t *data, uint32_t length)
{
    epd_set_dc_cs(epd, 1, 0);
    spi_write_blocking(epd->spi_port, data, length);
    epd_set_dc_cs(epd, 1, 1);
}

void epd_fill(epd_t *epd, uint8_t color)
{
    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    uint16_t row_bytes = (epd->width * bits_per_pixel + 7) / 8;
    uint16_t height = epd->height;

    uint8_t cmd = 0x10;
    epd_send_command(epd, cmd);

    uint8_t data = 0;
    const uint8_t pixels_per_byte = 8 / bits_per_pixel;
    const uint8_t pixel_mask = (1 << bits_per_pixel) - 1;
    for (uint8_t i = 0; i < pixels_per_byte; i++)
    {
        data |= (color & pixel_mask) << (i * bits_per_pixel);
    }

    uint8_t row_buffer[row_bytes];
    memset(row_buffer, data, row_bytes);

    epd_set_dc_cs(epd, 1, 0);
    for (uint16_t j = 0; j < height; j++)
    {
        spi_write_blocking(epd->spi_port, row_buffer, row_bytes);
    }
    epd_set_dc_cs(epd, 1, 1);

    epd_turn_on(epd);
}

void epd_display(epd_t *epd, uint8_t *data)
{
    const uint8_t bits_per_pixel = (uint8_t)epd->depth;
    uint16_t row_bytes = (epd->width * bits_per_pixel + 7) / 8;
    uint16_t height = epd->height;

    epd_send_command(epd, 0x10);
    epd_send_data(epd, data, (uint32_t)row_bytes * height);

    epd_turn_on(epd);
}

bool epd_read_busy_timeout(epd_t *epd, uint32_t timeout_ms)
{
    sleep_ms(10);
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (gpio_get(epd->pin_cfg.busy) == 0)
    {
        if (time_reached(deadline))
        {
            return false;
        }
        sleep_ms(10);
    }
    return true;
}

void epd_read_busy(epd_t *epd)
{
    epd_read_busy_timeout(epd, EPD_BUSY_DEFAULT_TIMEOUT_MS);
}

void epd_reset(epd_t *epd)
{
    gpio_put(epd->pin_cfg.rst, 0);
    sleep_ms(100);
    gpio_put(epd->pin_cfg.rst, 1);
    sleep_ms(100);
}

void epd_set_dc_cs(epd_t *epd, uint8_t dc, uint8_t cs)
{
    sleep_us(1);
    gpio_put_masked(epd->gpio_mask, (dc << epd->pin_cfg.dc) | (cs << epd->pin_cfg.cs));
    sleep_us(1);
}

void epd_sleep(epd_t *epd)
{
    const uint8_t data = 0xa5;

    epd_send_command(epd, 0x02);
    epd_read_busy(epd);

    sleep_ms(100);

    epd_send_command(epd, 0x07);
    epd_send_data(epd, &data, sizeof(uint8_t));
}

