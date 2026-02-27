/*
 * w25q64.c
 *
 *  Created on: Feb 25, 2026
 *      Author: Rubin Khadka
 */

#include "w25q64.h"
#include "spi1.h"
#include "dwt.h"
#include "lcd.h"
#include "uart.h"

uint8_t W25Q64_ReadID(void)
{
  uint8_t id;

  SPI1_CS_Low();
  DWT_Delay_us(10);

  SPI1_Transfer(W25Q64_CMD_READ_ID);
  id = SPI1_Transfer(0xFF);   // First byte is manufacturer ID
  SPI1_Transfer(0xFF);        // Memory Types
  SPI1_Transfer(0xFF);        // Capacity

  SPI1_CS_High();

  return id;
}

void W25Q64_Init(void)
{
  uint8_t w25q64_id = 0;
  w25q64_id = W25Q64_ReadID();

  if(w25q64_id == 0xEF)  // Winbond manufacturer ID
  {
    LCD_Clear();
    LCD_SendString("W25Q64 Flash");
    LCD_SetCursor(1, 0);
    LCD_SendString("ID: 0xEF");

    // Optional: UART output too
    USART1_SendString("W25Q64 Found!! ID: 0xEF\r\n");
  }
  else
  {
    LCD_Clear();
    LCD_SendString("W25Q64 ERROR!");
    LCD_SetCursor(1, 0);
    LCD_SendString("ID: ");

    // Convert ID to hex for display
    char hex[3];
    hex[0] = "0123456789ABCDEF"[w25q64_id >> 4];
    hex[1] = "0123456789ABCDEF"[w25q64_id & 0x0F];
    hex[2] = 0;
    LCD_SendString(hex);

    USART1_SendString("W25Q64 Error! ID: 0x");
    USART1_SendString(hex);
    USART1_SendString("\r\n");
  }
}

uint8_t W25Q64_ReadStatus(void)
{
  uint8_t status;

  SPI1_CS_Low();
  DWT_Delay_us(10);

  SPI1_Transfer(W25Q64_CMD_READ_STATUS);
  status = SPI1_Transfer(0xFF);

  SPI1_CS_High();

  return status;
}

void W25Q64_WaitBusy(void)
{
  // Wait until BUSY bit clears
  while(W25Q64_ReadStatus() & W25Q64_SR_BUSY);
}

void W25Q64_WriteEnable(void)
{
  SPI1_CS_Low();
  SPI1_Transfer(W25Q64_CMD_WRITE_ENABLE);
  SPI1_CS_High();
}

void W25Q64_WriteDisable(void)
{
  SPI1_CS_Low();
  SPI1_Transfer(W25Q64_CMD_WRITE_DISABLE);
  SPI1_CS_High();
}

void W25Q64_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
  uint32_t i;

  SPI1_CS_Low();

  // Send read command
  SPI1_Transfer(W25Q64_CMD_READ_DATA);

  // Send 24-bit address (MSB first)
  SPI1_Transfer((addr >> 16) & 0xFF);
  SPI1_Transfer((addr >> 8) & 0xFF);
  SPI1_Transfer(addr & 0xFF);

  // Read data
  for(i = 0; i < len; i++)
  {
    buf[i] = SPI1_Transfer(0xFF);
  }

  SPI1_CS_High();
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

  SPI1_CS_Low();

  // Send page program command
  SPI1_Transfer(W25Q64_CMD_PAGE_PROGRAM);

  // Send 24-bit address
  SPI1_Transfer((addr >> 16) & 0xFF);
  SPI1_Transfer((addr >> 8) & 0xFF);
  SPI1_Transfer(addr & 0xFF);

  // Write data
  for(i = 0; i < len; i++)
  {
    SPI1_Transfer(buf[i]);
  }

  SPI1_CS_High();

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

  SPI1_CS_Low();

  SPI1_Transfer(W25Q64_CMD_SECTOR_ERASE);
  SPI1_Transfer((addr >> 16) & 0xFF);
  SPI1_Transfer((addr >> 8) & 0xFF);
  SPI1_Transfer(addr & 0xFF);

  SPI1_CS_High();

  W25Q64_WaitBusy();  // Wait for erase to complete
}

void W25Q64_EraseBlock32K(uint32_t addr)
{
  W25Q64_WriteEnable();
  W25Q64_WaitBusy();

  SPI1_CS_Low();

  SPI1_Transfer(W25Q64_CMD_BLOCK_ERASE_32K);
  SPI1_Transfer((addr >> 16) & 0xFF);
  SPI1_Transfer((addr >> 8) & 0xFF);
  SPI1_Transfer(addr & 0xFF);

  SPI1_CS_High();

  W25Q64_WaitBusy();
}

void W25Q64_EraseBlock64K(uint32_t addr)
{
  W25Q64_WriteEnable();
  W25Q64_WaitBusy();

  SPI1_CS_Low();

  SPI1_Transfer(W25Q64_CMD_BLOCK_ERASE_64K);
  SPI1_Transfer((addr >> 16) & 0xFF);
  SPI1_Transfer((addr >> 8) & 0xFF);
  SPI1_Transfer(addr & 0xFF);

  SPI1_CS_High();

  W25Q64_WaitBusy();
}

void W25Q64_EraseChip(void)
{
  W25Q64_WriteEnable();
  W25Q64_WaitBusy();

  SPI1_CS_Low();
  SPI1_Transfer(W25Q64_CMD_CHIP_ERASE);
  SPI1_CS_High();

  W25Q64_WaitBusy();  // This takes several seconds!
}
