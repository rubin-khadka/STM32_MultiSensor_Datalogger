/*
 * spi1.h
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#ifndef SPI1_H_
#define SPI1_H_

// Function Prototypes
void SPI1_Init(void);
void SPI1_CS_Low(void);
void SPI1_CS_High(void);
uint8_t SPI1_Transfer(uint8_t data);

#endif /* SPI1_H_ */
