/*
 * w25q64.h
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#ifndef W25Q64_H_
#define W25Q64_H_

#include <stdint.h>

// Commands
#define W25Q64_CMD_WRITE_ENABLE     0x06
#define W25Q64_CMD_WRITE_DISABLE    0x04
#define W25Q64_CMD_READ_STATUS      0x05
#define W25Q64_CMD_READ_DATA        0x03
#define W25Q64_CMD_FAST_READ        0x0B
#define W25Q64_CMD_PAGE_PROGRAM     0x02
#define W25Q64_CMD_SECTOR_ERASE     0x20
#define W25Q64_CMD_BLOCK_ERASE_32K  0x52
#define W25Q64_CMD_BLOCK_ERASE_64K  0xD8
#define W25Q64_CMD_CHIP_ERASE       0xC7
#define W25Q64_CMD_READ_ID          0x9F

// Status register bits
#define W25Q64_SR_BUSY       (1 << 0)
#define W25Q64_SR_WEL        (1 << 1) // Write Enable Latch

// Memory organization
#define W25Q64_PAGE_SIZE      256
#define W25Q64_SECTOR_SIZE    4096
#define W25Q64_BLOCK_SIZE_32K 32768
#define W25Q64_BLOCK_SIZE_64K 65536
#define W25Q64_TOTAL_SIZE     8388608  // 8MB

// Function Prototypes
uint8_t W25Q64_ReadID(void);
void W25Q64_CheckPresence(void);
uint8_t W25Q64_ReadStatus(void);
void W25Q64_WaitBusy(void);
void W25Q64_WriteEnable(void);
void W25Q64_WriteDisable(void);

// Read/Write functions
void W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len);
void W25Q64_WritePage(uint32_t addr, const uint8_t *buf, uint32_t len);
void W25Q64_Write(uint32_t addr, const uint8_t *buf, uint32_t len);

// Erase functions
void W25Q64_EraseSector(uint32_t addr);
void W25Q64_EraseBlock32K(uint32_t addr);
void W25Q64_EraseBlock64K(uint32_t addr);
void W25Q64_EraseChip(void);

#endif /* W25Q64_H_ */
