/*
 * i2c_lcd.c
 *
 *  Created on: Feb 21, 2026
 *      Author: Rubin Khadka
 */

#include "lcd.h"
#include "timer2.h"  // For delays
#include "utils.h"

// PCF8574 bits
#define LCD_BACKLIGHT   0x08
#define LCD_ENABLE      0x04
#define LCD_RW          0x02
#define LCD_RS          0x01

// Send 4 bits to LCD
static void LCD_SendNibble(uint8_t nibble, uint8_t rs)
{
  uint8_t data;

  // Combine nibble with control bits
  data = nibble | LCD_BACKLIGHT;  // Backlight always on

  if(rs)
    data |= LCD_RS;  // Set RS for data

  // ENABLE HIGH
  I2C1_Start();
  I2C1_SendAddr(LCD_ADDR, I2C_WRITE);
  I2C1_WriteByte(data | LCD_ENABLE);  // E=1
  I2C1_Stop();

  // Small pulse delay
  for(volatile int i = 0; i < 10; i++);

  // ENABLE LOW
  I2C1_Start();
  I2C1_SendAddr(LCD_ADDR, I2C_WRITE);
  I2C1_WriteByte(data & ~LCD_ENABLE);  // E=0
  I2C1_Stop();
}

// Send command (RS=0)
void LCD_SendCmd(uint8_t cmd)
{
  // Send high nibble first
  LCD_SendNibble(cmd & 0xF0, 0);

  // Send low nibble
  LCD_SendNibble((cmd << 4) & 0xF0, 0);

  // Commands need execution time
  if(cmd == 0x01 || cmd == 0x02)
    TIMER2_Delay_ms(20);
  else
    TIMER2_Delay_ms(1);
}

// Send data (RS=1)
void LCD_SendData(uint8_t data)
{
  // Send high nibble first
  LCD_SendNibble(data & 0xF0, 1);

  // Send low nibble
  LCD_SendNibble((data << 4) & 0xF0, 1);

  for(volatile int i = 0; i < 500; i++);
}

// Initialize LCD
void LCD_Init(void)
{
  // Power-up delay
  TIMER2_Delay_ms(100);

  // Reset sequence (from HD44780 datasheet)
  LCD_SendNibble(0x30, 0);  // 8-bit mode
  TIMER2_Delay_ms(5);

  LCD_SendNibble(0x30, 0);  // 8-bit mode again
  TIMER2_Delay_ms(5);

  LCD_SendNibble(0x30, 0);  // 8-bit mode again
  TIMER2_Delay_ms(5);

  LCD_SendNibble(0x20, 0);  // Switch to 4-bit mode
  TIMER2_Delay_ms(5);

  // Now in 4-bit mode, send configuration commands
  LCD_SendCmd(0x28);  // 2 lines, 5x8 font
  LCD_SendCmd(0x08);  // Display off
  LCD_SendCmd(0x01);  // Clear display
  LCD_SendCmd(0x06);  // Entry mode
  LCD_SendCmd(0x0C);  // Display on, cursor off
}

// Send string
void LCD_SendString(char *str)
{
  while(*str)
  {
    LCD_SendData(*str++);
  }
}

// Clear display
void LCD_Clear(void)
{
  LCD_SendCmd(0x01);
}

// Set cursor position
void LCD_SetCursor(uint8_t row, uint8_t col)
{
  uint8_t address;

  if(row == 0)
    address = 0x80 + col;
  else
    address = 0xC0 + col;

  LCD_SendCmd(address);
}

void LCD_DisplayError(void)
{
  LCD_Clear();

  LCD_SetCursor(0, 0);
  LCD_SendString("TEMP: ERROR    ");

  LCD_SetCursor(1, 0);
  LCD_SendString("HUMD: ERROR    ");

}

void LCD_DisplayReading(uint8_t temp_int, uint8_t temp_dec, uint8_t hum_int, uint8_t hum_dec)
{
  // LINE 1: TEMP: XX.X C
  LCD_SetCursor(0, 0);
  LCD_SendString("TEMP: ");

  // Format temperature: XX.X
  if(temp_int >= 10)
  {
    LCD_SendData('0' + (temp_int / 10));
    LCD_SendData('0' + (temp_int % 10));
  }
  else
  {
    LCD_SendData(' ');
    LCD_SendData('0' + temp_int);
  }

  LCD_SendData('.');
  LCD_SendData('0' + temp_dec);
  LCD_SendData(' ');
  LCD_SendData('C');
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');

  // LINE 2: HUMD: XX.X %
  LCD_SetCursor(1, 0);
  LCD_SendString("HUMD: ");

  // Format humidity: XX.X
  if(hum_int >= 10)
  {
    LCD_SendData('0' + (hum_int / 10));
    LCD_SendData('0' + (hum_int % 10));
  }
  else
  {
    LCD_SendData(' ');
    LCD_SendData('0' + hum_int);
  }

  LCD_SendData('.');
  LCD_SendData('0' + hum_dec);
  LCD_SendData(' ');
  LCD_SendData('%');
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');

}

void LCD_DisplayAccel(int16_t ax, int16_t ay, int16_t az)
{
  char buf[8];

  // Line 1: AX and AY
  LCD_SetCursor(0, 0);
  LCD_SendString("AX:");
  itoa_16(ax, buf);
  LCD_SendString(buf);
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');

  LCD_SetCursor(0, 8);
  LCD_SendString(" AY:");
  itoa_16(ay, buf);
  LCD_SendString(buf);

  // Line 2: AZ
  LCD_SetCursor(1, 0);
  LCD_SendString("AZ:");
  itoa_16(az, buf);
  LCD_SendString(buf);
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');
}

void LCD_DisplayGyro(int16_t gx, int16_t gy, int16_t gz)
{
  char buf[8];

  // Line 1: GX and GY
  LCD_SetCursor(0, 0);
  LCD_SendString("GX:");
  itoa_16(gx, buf);
  LCD_SendString(buf);
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');

  LCD_SetCursor(0, 8);
  LCD_SendString(" GY:");
  itoa_16(gy, buf);
  LCD_SendString(buf);

  // Line 2: GZ
  LCD_SetCursor(1, 0);
  LCD_SendString("GZ:");
  itoa_16(gz, buf);
  LCD_SendString(buf);
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');
  LCD_SendData(' ');
}

// Helper function to display float on LCD with specified decimal places
void LCD_DisplayFloat(float value, uint8_t decimal_places)
{
  // Handle negative values
  if(value < 0)
  {
    LCD_SendData('-');
    value = -value;
  }

  // Get integer part
  uint16_t int_part = (uint16_t) value;

  // Get fractional part
  float fractional = value - int_part;
  uint16_t frac_part = 0;

  // Convert fractional to integer (e.g., 0.123 -> 123 for 3 decimal places)
  for(uint8_t i = 0; i < decimal_places; i++)
  {
    fractional *= 10;
  }
  frac_part = (uint16_t) (fractional + 0.5);  // Add 0.5 for rounding

  // Handle case where rounding increased the fractional part
  if(frac_part >= 1000 && decimal_places == 3)
  {
    frac_part = 0;
    int_part++;
  }
  else if(frac_part >= 100 && decimal_places == 2)
  {
    frac_part = 0;
    int_part++;
  }
  else if(frac_part >= 10 && decimal_places == 1)
  {
    frac_part = 0;
    int_part++;
  }

  // Display integer part with proper spacing for 3 digits
  if(int_part >= 100)
  {
    LCD_SendData('0' + (int_part / 100));
    LCD_SendData('0' + ((int_part / 10) % 10));
    LCD_SendData('0' + (int_part % 10));
  }
  else if(int_part >= 10)
  {
    LCD_SendData(' ');
    LCD_SendData('0' + (int_part / 10));
    LCD_SendData('0' + (int_part % 10));
  }
  else
  {
    LCD_SendData(' ');
    LCD_SendData(' ');
    LCD_SendData('0' + int_part);
  }

  // Decimal point
  LCD_SendData('.');

  // Display fractional part with leading zeros
  if(decimal_places == 3)
  {
    LCD_SendData('0' + (frac_part / 100));
    LCD_SendData('0' + ((frac_part / 10) % 10));
    LCD_SendData('0' + (frac_part % 10));
  }
  else if(decimal_places == 2)
  {
    LCD_SendData('0' + (frac_part / 10));
    LCD_SendData('0' + (frac_part % 10));
  }
  else if(decimal_places == 1)
  {
    LCD_SendData('0' + frac_part);
  }
}

// Display scaled accelerometer data on LCD
void LCD_DisplayAccelScaled(float ax, float ay, float az)
{
  // Line 1: AX and AY with units
  LCD_SetCursor(0, 0);
  LCD_SendString("AX:");
  LCD_DisplayFloat(ax, 2);  // 2 decimal places
  LCD_SendData('g');
  LCD_SendData(' ');

  // Clear remaining spaces on line 1 if needed
  LCD_SetCursor(0, 10);  // Position after "AX:1.23g "
  LCD_SendString("AY:");
  LCD_DisplayFloat(ay, 2);
  LCD_SendData('g');

  // Fill rest of line with spaces if needed
  LCD_SetCursor(0, 19);
  LCD_SendData(' ');

  // Line 2: AZ
  LCD_SetCursor(1, 0);
  LCD_SendString("AZ:");
  LCD_DisplayFloat(az, 2);
  LCD_SendData('g');

  // Clear rest of line 2
  for(uint8_t i = 6; i < 20; i++)  // Assuming 20 char LCD
  {
    LCD_SetCursor(1, i);
    LCD_SendData(' ');
  }
}

// Display scaled gyroscope data on LCD
void LCD_DisplayGyroScaled(float gx, float gy, float gz)
{
  // Line 1: GX and GY with units
  LCD_SetCursor(0, 0);
  LCD_SendString("GX:");
  LCD_DisplayFloat(gx, 1);  // 1 decimal place for gyro
  LCD_SendData('d');
  LCD_SendData('p');
  LCD_SendData('s');

  LCD_SetCursor(0, 10);
  LCD_SendString("GY:");
  LCD_DisplayFloat(gy, 1);
  LCD_SendData('d');
  LCD_SendData('p');
  LCD_SendData('s');

  // Fill rest of line with spaces
  LCD_SetCursor(0, 19);
  LCD_SendData(' ');

  // Line 2: GZ
  LCD_SetCursor(1, 0);
  LCD_SendString("GZ:");
  LCD_DisplayFloat(gz, 1);
  LCD_SendData('d');
  LCD_SendData('p');
  LCD_SendData('s');

  // Clear rest of line 2
  for(uint8_t i = 8; i < 20; i++)
  {
    LCD_SetCursor(1, i);
    LCD_SendData(' ');
  }
}

