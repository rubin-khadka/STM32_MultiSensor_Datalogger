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

#define MPU_READ_TICKS      5
#define LCD_UPDATE_TICKS    10
#define UART_UPDATE_TICKS   10

int main(void)
{

  // Initialize ALL modules
  TIMER2_Init();    // 0.1ms resolution for long delays
  USART1_Init();    // UART for output
  I2C1_Init();
  LCD_Init();       // LCD for display
  MPU6050_Init();
  Button_Init();
  TIMER4_Init();

  // Loop counters
  uint8_t mpu_count = 0;
  uint8_t lcd_count = 0;
  uint8_t uart_count = 0;

  LCD_Clear();
  LCD_SendString("STM32 PROJECT");
  LCD_SetCursor(1, 0);
  LCD_SendString("INITIALIZING...");

  TIMER2_Delay_ms(2000);

  // Setup TIM3 for 10ms control loop
  TIMER3_SetupPeriod(10);  // 10ms period

  while(1)
  {
    // Run tasks at different rates
    // Read MPU6050 every 50ms
    if(mpu_count++ >= MPU_READ_TICKS)
    {
      Task_MPU6050_Read();
      mpu_count = 0;
    }

    // Updata LCD every 100ms
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
