#include "enc28j60_spi.h"
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>

#define ENC28J60_SPI        SPI1
#define ENC28J60_CS_PORT    GPIOA
#define ENC28J60_CS_PIN     GPIO4

void enc28j60_spi_init_lomc3(void) {
    // Включаем тактирование SPI1 и GPIO
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_GPIOA);

    // Настройка GPIO для SPI1 (PA5-SCK, PA6-MISO, PA7-MOSI)
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO5 | GPIO7);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO6);

    // Настройка CS (Chip Select) - PA4
    gpio_set_mode(ENC28J60_CS_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, ENC28J60_CS_PIN);
    gpio_set(ENC28J60_CS_PORT, ENC28J60_CS_PIN); // Отключаем CS

    // Настройка SPI1 (Master, Fclk/16, CPOL=0, CPHA=0)
    spi_reset(ENC28J60_SPI);
    spi_init_master(ENC28J60_SPI, SPI_CR1_BAUDRATE_FPCLK_DIV_16, SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
                    SPI_CR1_CPHA_CLK_TRANSITION_1, SPI_CR1_DFF_8BIT, SPI_CR1_MSBFIRST);
    spi_enable(ENC28J60_SPI);
}

void enc28j60_select_lomc3(void) {
    gpio_clear(ENC28J60_CS_PORT, ENC28J60_CS_PIN);
}

void enc28j60_deselect_lomc3(void) {
    gpio_set(ENC28J60_CS_PORT, ENC28J60_CS_PIN);
}

uint8_t enc28j60_spi_transfer_lomc3(uint8_t data) {
    spi_send(ENC28J60_SPI, data);
    return spi_read(ENC28J60_SPI);
}
