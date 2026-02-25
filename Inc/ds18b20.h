/*
 * ds18b20.h
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#ifndef DS18B20_H_
#define DS18B20_H_

#include "stdint.h"

// Function Prototypes
void DS18B20_Init(void);
int DS18B20_Reset(void);
void DS18B20_WriteByte(uint8_t data);
uint8_t DS18B20_ReadByte(void);
float DS18B20_ReadTemperature(void);

#endif /* DS18B20_H_ */
