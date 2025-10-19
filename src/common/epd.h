#pragma once
#include "hardware/pwm.h"
#include "hardware/spi.h"

/*************************************/
/*             CONSTANT              */
/*************************************/
#define OFFSET_CMD (uint16_t)1
#define OFFSET_DATA (uint16_t)2

#define BAUD_RATE 1000000

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
    spi_inst_t *spi_port;
} epd_t;

/*************************************/
/*      FUNCION DECLARATION          */
/*************************************/
epd_t epd_create(uint8_t dc);
void epd_init(epd_t *epd);
void epd_turn_on(epd_t *epd);
void epd_fill(epd_t *epd, uint8_t color);
void epd_display(epd_t *epd, uint8_t *canvas);
void epd_send_command(epd_t *epd, uint8_t command);
void epd_send_data(epd_t *epd, const uint8_t *data, uint32_t length);
void epd_set_dc_cs(epd_t *epd, uint8_t dc, uint8_t cs);
void epd_read_busy(epd_t *epd);
void epd_reset(epd_t *epd);
void epd_sleep(epd_t *epd);