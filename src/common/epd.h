#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "hardware/pwm.h"
#include "hardware/spi.h"

/*************************************/
/*             CONSTANT              */
/*************************************/
#define OFFSET_CMD (uint16_t)1
#define OFFSET_DATA (uint16_t)2

#define BAUD_RATE 4000000

/*************************************/
/*              ENUMS                */
/*************************************/
typedef enum {
    DEPTH_1BIT = 1,
    DEPTH_2BIT = 2,
    DEPTH_3BIT = 3,
    DEPTH_4BIT = 4,
    DEPTH_8BIT = 8, 
} depth_t;

typedef enum
{
    EPD_ROT_0 = 0,
    EPD_ROT_90,
    EPD_ROT_180,
    EPD_ROT_270,
} epd_rotation_t;

/*************************************/
/*         DATA STRUCTURE            */
/*************************************/

typedef struct
{
    uint8_t sck;
    uint8_t mosi;
    uint8_t cs;
    uint8_t dc;
    uint8_t rst;
    uint8_t busy;
} pin_cfg_t;

typedef struct epd_t
{
    uint32_t baud_rate;
    uint32_t gpio_mask;
    uint16_t width;
    uint16_t height;
    pin_cfg_t pin_cfg;
    depth_t depth;
    epd_rotation_t rotation;
    spi_inst_t *spi_port;
} epd_t;

/*************************************/
/*       FUNCTION DECLARATION        */
/*************************************/
epd_t epd_create(uint8_t dc);
epd_t epd_create_custom(spi_inst_t *spi, pin_cfg_t pin_cfg, uint16_t width, uint16_t height, depth_t depth, uint32_t baud_rate);
void epd_init(epd_t *epd);
void epd_turn_on(epd_t *epd);
void epd_fill(epd_t *epd, uint8_t color);
void epd_display(epd_t *epd, uint8_t *canvas);
void epd_send_command(epd_t *epd, uint8_t command);
void epd_send_data(epd_t *epd, const uint8_t *data, uint32_t length);
void epd_set_dc_cs(epd_t *epd, uint8_t dc, uint8_t cs);
void epd_read_busy(epd_t *epd);
bool epd_read_busy_timeout(epd_t *epd, uint32_t timeout_ms);
void epd_reset(epd_t *epd);
void epd_sleep(epd_t *epd);

