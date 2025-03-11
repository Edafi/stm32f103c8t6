#ifndef ENC28J60_SPI_H
#define ENC28J60_SPI_H

#include <stdint.h>

void enc28j60_spi_init_lomc3(void);
uint8_t enc28j60_spi_transfer_lomc3(uint8_t data);
void enc28j60_select_lomc3(void);
void enc28j60_deselect_lomc3(void);

#endif // ENC28J60_SPI_H
