/*
 * w25q64.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#include "w25q64.h"
#include "spi1.h"
#include "dwt.h"

uint32_t W25Q64_ReadID(void)
{
  uint8_t id[3] = {0};

  SPI1_CS_Low();
  DWT_Delay_us(10);

  SPI1_Transfer(0x9F);  // Read ID command

  id[0] = SPI1_Transfer(0xFF);
  id[1] = SPI1_Transfer(0xFF);
  id[2] = SPI1_Transfer(0xFF);

  SPI1_CS_High();

  return (id[0] << 16) | (id[1] << 8) | id[2];
}
