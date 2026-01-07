#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include "driver/gpio.h"

// DS18B20 Configuration
#define DS18B20_PIN GPIO_NUM_4  // Change this to your actual pin
#define DS18B20_FAMILY_CODE 0x28
#define DS18B20_RESOLUTION_12BIT 0x7F

/**
 * @brief Initialize DS18B20 sensor
 */
void ds18b20_init(void);

/**
 * @brief Read temperature from DS18B20 sensor
 * @return Temperature in Celsius degrees
 */
float ds18b20_read_temperature(void);

/**
 * @brief Check if DS18B20 sensor is present on the bus
 * @return true if sensor is present, false otherwise
 */
bool ds18b20_is_present(void);

#endif // DS18B20_H