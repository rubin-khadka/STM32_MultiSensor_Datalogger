/*
 * utils.h
 *
 *  Created on: Feb 20, 2026
 *      Author: Rubin Khadka
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

// Function Prototypes
// Integer to string utility functions to display raw values
void itoa_16(int16_t value, char *buffer);
void itoa_8(uint8_t value, char *buffer);
void format_value(uint8_t integer, uint8_t decimal, char *buffer, char unit);
void format_reading(uint8_t temp_int, uint8_t temp_dec, uint8_t hum_int, uint8_t hum_dec, char *buffer);
void format_temp_hum(char *buffer, uint8_t temp, uint8_t hum);
void format_accel(char *buffer, int16_t ax, int16_t ay, int16_t az);
void format_gyro(char *buffer, int16_t gx, int16_t gy, int16_t gz);

// Float to string utility functions to display scaled values
void ftoa(float value, char *buffer, uint8_t decimal_places);
void format_float(float value, char *buffer, uint8_t decimal_places, char unit);
void format_accel_scaled(char *buffer, float ax, float ay, float az, uint8_t decimal_places);
void format_gyro_scaled(char *buffer, float gx, float gy, float gz, uint8_t decimal_places);

#endif /* UTILS_H_ */
