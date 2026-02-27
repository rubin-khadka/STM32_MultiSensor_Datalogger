/*
 * logger.h
 *
 *  Created on: Feb 26, 2026
 *      Author: Rubin Khadka
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "stdint.h"

// Log entry structure
typedef struct
{
  int16_t ds18b20_temp;     // DS18B20
  int16_t mpu_temp;         // MPU6050

  // Accelerometer
  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;

  // Gyroscope
  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;

  // Sequence number, add real timestamp in another project
  uint16_t sequence;
} __attribute__((packed)) LogEntry_t;

// Public functions
void Logger_Init(void);
void Logger_SaveEntry(void);
void Logger_DumpAll(void);
void Logger_EraseAll(void);
uint32_t Logger_GetEntryCount(void);

#endif /* LOGGER_H_ */
