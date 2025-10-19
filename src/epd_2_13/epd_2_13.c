#include "epd_2_13.h"
#include "epd.h"

epd_t epd_create(uint8_t dc)
{
    pin_cfg_t pin_cfg;
    epd_t epd;

    pin_cfg.cs = EPD_2_13_PIN_CS;
    pin_cfg.dc = dc;
    pin_cfg.mosi = EPD_2_13_PIN_MOSI;
    pin_cfg.rst = EPD_2_13_PIN_RESET;
    pin_cfg.sck = EPD_2_13_PIN_CLK;
    pin_cfg.busy = EPD_2_13_PIN_BUSY;

    epd.width = EPD_2_13_WIDTH;
    epd.height = EPD_2_13_HEIGHT;
    epd.baud_rate = BAUD_RATE;
    epd.depth = DEPTH_2BIT;
    epd.spi_port = spi1;
    epd.pin_cfg = pin_cfg;
    epd.gpio_mask = (1u << epd.pin_cfg.dc) | (1u << epd.pin_cfg.cs);

    return epd;
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

    while (offset_ptr != sizeof(epd_2_13))
    {
        uint8_t a = *(epd_2_13 + offset_ptr);
        command = *(epd_2_13 + offset_ptr + OFFSET_CMD);
        length_data = *(epd_2_13 + offset_ptr);

        epd_send_command(epd, command);

        if (0 != length_data)
        {
            epd_send_data(epd, epd_2_13 + offset_ptr + OFFSET_DATA, length_data);
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
    epd_send_data(epd, &cmd, sizeof(uint8_t));
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

    uint16_t width = epd->width / 4;
    uint16_t height = epd->height;

    uint8_t cmd = 0x10;
    epd_send_command(epd, cmd);

    for (uint16_t j = 0; j < height; j++)
    {
        for (uint16_t i = 0; i < width; i++)
        {
            uint8_t data = (color << 6) | (color << 4) | (color << 2) | color;
            epd_send_data(epd, &data, 1);
        }
    }

    epd_turn_on(epd);
}

void epd_display(epd_t *epd, uint8_t *data)
{
    uint16_t width = epd->width / 4;
    uint16_t height = epd->height;

    epd_send_command(epd, 0x10);
    epd_send_data(epd, data, width * height);

    epd_turn_on(epd);
}

void epd_read_busy(epd_t *epd)
{
    sleep_ms(100);
    while (gpio_get(epd->pin_cfg.busy) == 0)
    {
        sleep_ms(100);
    }
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