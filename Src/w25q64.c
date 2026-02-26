/*
 * w25q64.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#include "w25q64.h"
#include "spi1.h"
#include "uart.h"
#include "dwt.h"
#include "stdio.h"

void W25Q64_Init(void)
{
  W25Q64_CS_HIGH();
}

void W25Q64_Reset(void)
{
    USART1_SendString("Resetting W25Q64...\r\n");

    W25Q64_CS_LOW();

    // Send both bytes WITHOUT any delay between them
    SPI1_Transmit(0x66);  // Enable Reset
    SPI1_Transmit(0x99);  // Reset - send immediately after first byte

    W25Q64_CS_HIGH();

    DWT_Delay_ms(100);    // Wait for reset to complete (this delay AFTER CS high is fine)

    USART1_SendString("Reset complete\r\n");
}

uint32_t W25Q64_ReadJEDEC_ID(void)
{
    uint32_t id = 0;
    uint8_t rx[3];

    USART1_SendString("Reading full JEDEC ID...\r\n");
    W25Q64_CS_LOW();

    SPI1_Transmit(0x9F);  // Send command

    // Read 3 bytes continuously
    for(int i = 0; i < 3; i++)
    {
        rx[i] = SPI1_Transmit(0xFF);
    }

    W25Q64_CS_HIGH();

    id = (rx[0] << 16) | (rx[1] << 8) | rx[2];

    return id;
}

uint8_t W25Q64_ReadStatus(void)
{
  uint8_t status;

  W25Q64_CS_LOW();

  SPI1_Transmit(W25Q64_CMD_READ_STATUS);
  status = SPI1_Transmit(0xFF);

  W25Q64_CS_HIGH();

  return status;
}

void W25Q64_WaitBusy(void)
{
  // Wait until BUSY bit clears
  while(W25Q64_ReadStatus() & W25Q64_SR_BUSY);
}

void W25Q64_WriteEnable(void)
{
  W25Q64_CS_LOW();
  SPI1_Transmit(W25Q64_CMD_WRITE_ENABLE);
  W25Q64_CS_HIGH();
}

void W25Q64_WriteDisable(void)
{
  W25Q64_CS_LOW();
  SPI1_Transmit(W25Q64_CMD_WRITE_DISABLE);
  W25Q64_CS_HIGH();
}

void W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
  uint32_t i;

  W25Q64_CS_LOW();

  // Send read command
  SPI1_Transmit(W25Q64_CMD_READ_DATA);

  // Send 24-bit address (MSB first)
  SPI1_Transmit((addr >> 16) & 0xFF);
  SPI1_Transmit((addr >> 8) & 0xFF);
  SPI1_Transmit(addr & 0xFF);

  // Read data
  for(i = 0; i < len; i++)
  {
    buf[i] = SPI1_Transmit(0xFF);
  }

  W25Q64_CS_HIGH();
}

void W25Q64_WritePage(uint32_t addr, const uint8_t *buf, uint32_t len)
{
  uint32_t i;

  // Can't write more than one page
  if(len > W25Q64_PAGE_SIZE)
  {
    len = W25Q64_PAGE_SIZE;
  }

  // Make sure we don't cross page boundary
  if((addr & 0xFF) + len > W25Q64_PAGE_SIZE)
  {
    len = W25Q64_PAGE_SIZE - (addr & 0xFF);
  }

  W25Q64_WriteEnable();
  W25Q64_WaitBusy();

  W25Q64_CS_LOW();

  // Send page program command
  SPI1_Transmit(W25Q64_CMD_PAGE_PROGRAM);

  // Send 24-bit address
  SPI1_Transmit((addr >> 16) & 0xFF);
  SPI1_Transmit((addr >> 8) & 0xFF);
  SPI1_Transmit(addr & 0xFF);

  // Write data
  for(i = 0; i < len; i++)
  {
    SPI1_Transmit(buf[i]);
  }

  W25Q64_CS_HIGH();

  // Wait for programming to complete
  W25Q64_WaitBusy();
}

void W25Q64_Write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
  uint32_t remaining = len;
  uint32_t offset = 0;
  uint32_t page_offset = addr & 0xFF;
  uint32_t current_addr = addr;
  uint32_t chunk_size;

  while(remaining > 0)
  {
    // Calculate how many bytes we can write in this page
    chunk_size = W25Q64_PAGE_SIZE - page_offset;
    if(chunk_size > remaining)
    {
      chunk_size = remaining;
    }

    // Write one page
    W25Q64_WritePage(current_addr, &buf[offset], chunk_size);

    // Update pointers
    offset += chunk_size;
    current_addr += chunk_size;
    remaining -= chunk_size;
    page_offset = 0;  // After first page, all writes start at page boundary
  }
}

void W25Q64_EraseSector(uint32_t addr)
{
  W25Q64_WriteEnable();
  W25Q64_WaitBusy();

  W25Q64_CS_LOW();

  SPI1_Transmit(W25Q64_CMD_SECTOR_ERASE);
  SPI1_Transmit((addr >> 16) & 0xFF);
  SPI1_Transmit((addr >> 8) & 0xFF);
  SPI1_Transmit(addr & 0xFF);

  W25Q64_CS_HIGH();

  W25Q64_WaitBusy();  // Wait for erase to complete
}

void W25Q64_EraseBlock32K(uint32_t addr)
{
  W25Q64_WriteEnable();
  W25Q64_WaitBusy();

  W25Q64_CS_LOW();

  SPI1_Transmit(W25Q64_CMD_BLOCK_ERASE_32K);
  SPI1_Transmit((addr >> 16) & 0xFF);
  SPI1_Transmit((addr >> 8) & 0xFF);
  SPI1_Transmit(addr & 0xFF);

  W25Q64_CS_HIGH();

  W25Q64_WaitBusy();
}

void W25Q64_EraseBlock64K(uint32_t addr)
{
  W25Q64_WriteEnable();
  W25Q64_WaitBusy();

  W25Q64_CS_LOW();

  SPI1_Transmit(W25Q64_CMD_BLOCK_ERASE_64K);
  SPI1_Transmit((addr >> 16) & 0xFF);
  SPI1_Transmit((addr >> 8) & 0xFF);
  SPI1_Transmit(addr & 0xFF);

  W25Q64_CS_HIGH();

  W25Q64_WaitBusy();
}

void W25Q64_EraseChip(void)
{
  W25Q64_WriteEnable();
  W25Q64_WaitBusy();

  W25Q64_CS_LOW();
  SPI1_Transmit(W25Q64_CMD_CHIP_ERASE);
  W25Q64_CS_HIGH();

  W25Q64_WaitBusy();  // This takes several seconds!
}
