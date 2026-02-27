/*
 * logger.c
 *
 *  Created on: Feb 26, 2026
 *      Author: Rubin Khadka
 */

#include "logger.h"
#include "w25q64.h"
#include "ds18b20.h"
#include "mpu6050.h"
#include "lcd.h"
#include "uart.h"

// Memory layout
#define LOGGER_START_ADDR    (1 * W25Q64_SECTOR_SIZE)     // Start at sector 1
#define LOGGER_MAX_ADDR      (2047 * W25Q64_SECTOR_SIZE)  // Last sector
#define LOGGER_ENTRY_SIZE    sizeof(LogEntry_t)           // Should be 18

// Static variables
static uint32_t current_addr = LOGGER_START_ADDR;
static uint32_t sequence = 0;
static uint32_t entry_count = 0;

// Forward declarations
static void FindFirstEmptyLocation(void);
static void ShowMessage(const char *msg);

void Logger_Init(void)
{
  // Find where to start writing
  FindFirstEmptyLocation();

  // Show status
  char buf[16];
  sprintf(buf, "Entries: %lu", entry_count);

  LCD_Clear();
  LCD_SetCursor(0, 0);
  LCD_SendString("Logger Ready");
  LCD_SetCursor(1, 0);
  LCD_SendString(buf);

  USART1_SendString("Logger initialized. ");
  USART1_SendString(buf);
  USART1_SendString("\r\n");
}

void Logger_SaveEntry(void)
{
  LogEntry_t entry;

  // Check if flash is full
  if(current_addr >= LOGGER_MAX_ADDR)
  {
    ShowMessage("Flash Full!");
    return;
  }

  // Read all sensors
  entry.ds18b20_temp = ds18b20_data.valid ? (int16_t) (ds18b20_data.temperature * 100) : 0x7FFF;  // 0x7FFF = invalid
  entry.mpu_temp = (int16_t) (mpu6050_scaled.temp * 100);

  entry.accel_x = mpu6050_scaled.accel_x;
  entry.accel_y = mpu6050_scaled.accel_y;
  entry.accel_z = mpu6050_scaled.accel_z;

  entry.gyro_x = mpu6050_scaled.gyro_x;
  entry.gyro_y = mpu6050_scaled.gyro_y;
  entry.gyro_z = mpu6050_scaled.gyro_z;

  entry.sequence = ++sequence;

  // Write to flash
  W25Q64_Write(current_addr, (uint8_t*) &entry, LOGGER_ENTRY_SIZE);

  // Update pointers
  current_addr += LOGGER_ENTRY_SIZE;
  entry_count++;

  // Show feedback
  char buf[16];
  sprintf(buf, "Saved #%u", sequence);
  ShowMessage(buf);

  // Optional: Also send via UART for immediate verification
  USART1_SendString("Saved entry #");
  // Send sequence number
}

void Logger_DumpAll(void)
{
  LogEntry_t entry;
  uint32_t addr = LOGGER_START_ADDR;
  uint32_t count = 0;
  char buf[64];

  ShowMessage("Dumping...");

  // Send CSV header
  USART1_SendString("\r\n--- SENSOR LOG DUMP ---\r\n");
  USART1_SendString("Seq,DS18B20,MPU,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ\r\n");

  // Read and send all entries
  while(addr < current_addr && addr < LOGGER_MAX_ADDR)
  {
    W25Q64_Read(addr, (uint8_t*) &entry, LOGGER_ENTRY_SIZE);

    // Send as CSV
    sprintf(
        buf,
        "%u,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
        entry.sequence,
        entry.ds18b20_temp,
        entry.mpu_temp,
        entry.accel_x,
        entry.accel_y,
        entry.accel_z,
        entry.gyro_x,
        entry.gyro_y,
        entry.gyro_z);
    USART1_SendString(buf);

    count++;
    addr += LOGGER_ENTRY_SIZE;
  }

  // Send summary
  sprintf(buf, "Total: %lu entries\r\n", count);
  USART1_SendString("--- END ---\r\n");
  USART1_SendString(buf);

  // Show on LCD
  sprintf(buf, "Dumped %lu", count);
  ShowMessage(buf);
}

void Logger_EraseAll(void)
{
  ShowMessage("Erasing...");
  USART1_SendString("Erasing entire flash...\r\n");

  W25Q64_EraseChip();

  // Reset all pointers
  current_addr = LOGGER_START_ADDR;
  sequence = 0;
  entry_count = 0;

  ShowMessage("Flash Erased!");
  USART1_SendString("Flash erase complete!\r\n");
}

uint32_t Logger_GetEntryCount(void)
{
  return entry_count;
}

// Private helper functions
static void FindFirstEmptyLocation(void)
{
  uint8_t buffer[LOGGER_ENTRY_SIZE];
  uint32_t addr = LOGGER_START_ADDR;

  entry_count = 0;
  sequence = 0;

  // Scan through flash to find first empty spot
  while(addr < LOGGER_MAX_ADDR)
  {
    W25Q64_Read(addr, buffer, LOGGER_ENTRY_SIZE);

    // Check if this location is empty (all 0xFF)
    int empty = 1;
    for(int i = 0; i < LOGGER_ENTRY_SIZE; i++)
    {
      if(buffer[i] != 0xFF)
      {
        empty = 0;
        break;
      }
    }

    if(empty)
    {
      current_addr = addr;
      return;  // Found empty spot
    }

    // Not empty, move to next entry
    addr += LOGGER_ENTRY_SIZE;
    entry_count++;
  }

  // If we get here, flash is full
  current_addr = LOGGER_MAX_ADDR;
}

static void ShowMessage(const char *msg)
{
  LCD_Clear();
  LCD_SetCursor(0, 0);
  LCD_SendString("Logger");
  LCD_SetCursor(1, 0);
  LCD_SendString(msg);
}
