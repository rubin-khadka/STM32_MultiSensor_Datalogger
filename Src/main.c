/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Rubin Khadka
 * @brief          : STM32 Multi Sensor Data Logger Project main file
 ******************************************************************************
 */

#include "stm32f103xb.h"
#include "button.h"
#include "lcd.h"
#include "mpu6050.h"
#include "tasks.h"
#include "timer2.h"
#include "timer3.h"
#include "uart.h"
#include "utils.h"
#include "dwt.h"
#include "ds18b20.h"
#include "spi1.h"
#include "w25q64.h"
#include "logger.h"

#define DS18B20_READ_TICKS  100
#define MPU_READ_TICKS      5
#define LCD_UPDATE_TICKS    10
#define UART_UPDATE_TICKS   10

int main(void)
{
  // Initialize ALL modules
  TIMER2_Init();
  USART1_Init();
  I2C1_Init();
  LCD_Init();
  MPU6050_Init();
  Button_Init();
  TIMER4_Init();
  DWT_Init();
  DS18B20_Init();
  SPI1_Init();

  // Loop counters
  uint8_t ds18b20_count = 0;
  uint8_t mpu_count = 0;
  uint8_t lcd_count = 0;
  uint8_t uart_count = 0;

  LCD_Clear();
  LCD_SendString("STM32 PROJECT");
  LCD_SetCursor(1, 0);
  LCD_SendString("INITIALIZING...");

  DWT_Delay_ms(2000);

  W25Q64_Init();
  DWT_Delay_ms(2000);

  Logger_Init();
  DWT_Delay_ms(2000);

  // Setup TIM3 for 10ms control loop
  TIMER3_SetupPeriod(10);  // 10ms period

  while(1)
  {
    // Handle button
    if(g_button2_short)
    {
      g_button2_short = 0;
      // Logger_SaveEntry();
      Feedback_Show("Logger", "DATA SAVED", 1000);  // Show for 2 seconds
    }

    // Handle button 2 long press - Dump
    if(g_button2_long)
    {
      g_button2_long = 0;
      Feedback_Show("Logger", "DATA DUMPED", 1000);
    }

    // Handle button 3 long press - Erase
    if(g_button3_long)
    {
      g_button3_long = 0;
      Feedback_Show("Logger", "DATA ERASED", 1000);
    }

    // Update feedback timer (check if time expired)
    Task_Feedback_Update();
    // Run tasks at different rates

    // Read DS18B20 every 1 seconds
    if(ds18b20_count++ >= DS18B20_READ_TICKS)
    {
      Task_DS18B20_Read();
      ds18b20_count = 0;
    }

    // Read MPU6050 every 50ms
    if(mpu_count++ >= MPU_READ_TICKS)
    {
      Task_MPU6050_Read();
      mpu_count = 0;
    }

    // Update LCD every 100ms
    if(lcd_count++ >= LCD_UPDATE_TICKS)
    {
      Task_LCD_Update();
      lcd_count = 0;
    }

    // Update UART output every 100ms
    if(uart_count++ >= UART_UPDATE_TICKS)
    {
      Task_UART_Output();
      uart_count = 0;
    }

    TIMER3_WaitPeriod();
  }
}
