#ifndef ENC28J60_SPI_H
#define ENC28J60_SPI_H

#include <stdint.h>

void enc28j60_spi_init(void);
uint8_t enc28j60_spi_transfer(uint8_t data);
void enc28j60_select(void);
void enc28j60_deselect(void);

#endif // ENC28J60_SPI_H
