/*
 * w25q64.h
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#ifndef W25Q64_H_
#define W25Q64_H_

#include <stdint.h>

#define W25Q_CMD_READ_ID   0x9F

uint32_t W25Q64_ReadID(void);

#endif /* W25Q64_H_ */
