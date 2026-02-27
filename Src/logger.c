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
static void ultoa(uint32_t num, char *str);
static void send_string(const char *str);
static void send_int(int16_t num);
static void send_comma(void);
static void send_newline(void);

// String conversion and UART helpers
static void ultoa(uint32_t num, char *str)
{
  char temp[16];
  int i = 0, j = 0;

  // Handle 0 separately
  if(num == 0)
  {
    str[0] = '0';
    str[1] = '\0';
    return;
  }

  // Convert digits in reverse
  while(num > 0)
  {
    temp[i++] = '0' + (num % 10);
    num /= 10;
  }

  // Reverse to correct order
  while(i > 0)
  {
    str[j++] = temp[--i];
  }
  str[j] = '\0';
}

static void send_string(const char *str)
{
  while(*str)
  {
    // Assuming you have a USART1_SendChar function
    USART1_SendChar(*str++);
  }
}

static void send_int(int16_t num)
{
  char buf[8];

  if(num < 0)
  {
    USART1_SendChar('-');
    num = -num;
  }

  ultoa((uint32_t) num, buf);
  send_string(buf);
}

static void send_comma(void)
{
  USART1_SendChar(',');
}

static void send_newline(void)
{
  USART1_SendChar('\r');
  USART1_SendChar('\n');
}

// Logger Functions
void Logger_Init(void)
{
  char buf[16];

  // Find where to start writing
  FindFirstEmptyLocation();

  // Show status on LCD
  LCD_Clear();
  LCD_SetCursor(0, 0);
  LCD_SendString("Logger Ready");
  LCD_SetCursor(1, 0);
  LCD_SendString("Entries:");

  // Convert entry_count to string
  ultoa(entry_count, buf);

  // Position cursor after "Entries:" (assume 8 chars)
  LCD_SetCursor(1, 8);
  LCD_SendString(buf);

  // UART output
  send_string("Logger initialized. Entries: ");
  send_int(entry_count);
  send_newline();
}

void Logger_SaveEntry(void)
{
  LogEntry_t entry;
  char buf[16];

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

  // Show feedback on LCD
  LCD_Clear();
  LCD_SetCursor(0, 0);
  LCD_SendString("Saved #");

  // Convert sequence to string
  ultoa(sequence, buf);
  LCD_SendString(buf);

  LCD_SetCursor(1, 0);
  LCD_SendString("Logger");

  // Optional UART feedback
  send_string("Saved entry #");
  send_int(sequence);
  send_newline();
}

void Logger_DumpAll(void)
{
  LogEntry_t entry;
  uint32_t addr = LOGGER_START_ADDR;
  uint32_t count = 0;
  char buf[16];

  ShowMessage("Dumping...");

  // Send CSV header
  send_string("\r\n--- SENSOR LOG DUMP ---\r\n");
  send_string("Seq,DS18B20,MPU,AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ\r\n");

  // Read and send all entries
  while(addr < current_addr && addr < LOGGER_MAX_ADDR)
  {
    W25Q64_Read(addr, (uint8_t*) &entry, LOGGER_ENTRY_SIZE);

    // Send sequence
    send_int(entry.sequence);
    send_comma();

    // Send DS18B20 temp
    send_int(entry.ds18b20_temp);
    send_comma();

    // Send MPU temp
    send_int(entry.mpu_temp);
    send_comma();

    // Send accelerometer
    send_int(entry.accel_x);
    send_comma();
    send_int(entry.accel_y);
    send_comma();
    send_int(entry.accel_z);
    send_comma();

    // Send gyroscope
    send_int(entry.gyro_x);
    send_comma();
    send_int(entry.gyro_y);
    send_comma();
    send_int(entry.gyro_z);

    send_newline();

    count++;
    addr += LOGGER_ENTRY_SIZE;
  }

  send_string("--- END ---\r\n");
  send_string("Total: ");
  send_int(count);
  send_string(" entries\r\n");

  // Show on LCD
  buf[0] = 'D';
  buf[1] = 'u';
  buf[2] = 'm';
  buf[3] = 'p';
  buf[4] = 'e';
  buf[5] = 'd';
  buf[6] = ' ';
  ultoa(count, &buf[7]);
  ShowMessage(buf);
}

void Logger_EraseAll(void)
{
  ShowMessage("Erasing...");
  send_string("Erasing entire flash...\r\n");

  W25Q64_EraseChip();

  // Reset all pointers
  current_addr = LOGGER_START_ADDR;
  sequence = 0;
  entry_count = 0;

  ShowMessage("Flash Erased!");
  send_string("Flash erase complete!\r\n");
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
  LCD_SendString((char*)msg);
}
