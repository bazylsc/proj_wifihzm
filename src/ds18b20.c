#include "ds18b20.h"
#include "esp_log.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

static const char *DS18B20_TAG = "DS18B20";

// OneWire low-level functions
static void onewire_reset(void) {
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_PIN, 0);
    ets_delay_us(480);
    gpio_set_level(DS18B20_PIN, 1);
    ets_delay_us(70);
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
    ets_delay_us(410);
}

static bool onewire_presence_pulse(void) {
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_PIN, 0);
    ets_delay_us(480);
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
    ets_delay_us(70);
    bool present = !gpio_get_level(DS18B20_PIN);
    ets_delay_us(410);
    return present;
}

static void onewire_write_bit(int bit) {
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_PIN, 0);
    if (bit) {
        ets_delay_us(6);
        gpio_set_level(DS18B20_PIN, 1);
        ets_delay_us(64);
    } else {
        ets_delay_us(60);
        gpio_set_level(DS18B20_PIN, 1);
        ets_delay_us(10);
    }
}

static int onewire_read_bit(void) {
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_PIN, 0);
    ets_delay_us(6);
    gpio_set_level(DS18B20_PIN, 1);
    ets_delay_us(9);
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_INPUT);
    int bit = gpio_get_level(DS18B20_PIN);
    ets_delay_us(55);
    return bit;
}

static void onewire_write_byte(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        onewire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

static uint8_t onewire_read_byte(void) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        byte >>= 1;
        if (onewire_read_bit()) {
            byte |= 0x80;
        }
    }
    return byte;
}

// Public functions
void ds18b20_init(void) {
    gpio_reset_pin(DS18B20_PIN);
    gpio_set_direction(DS18B20_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DS18B20_PIN, 1);
    
    ESP_LOGI(DS18B20_TAG, "DS18B20 sensor initialized on GPIO %d", DS18B20_PIN);
    
    // Check if sensor is present
    if (ds18b20_is_present()) {
        ESP_LOGI(DS18B20_TAG, "DS18B20 sensor detected successfully");
    } else {
        ESP_LOGW(DS18B20_TAG, "No DS18B20 sensor detected - check wiring");
    }
}

bool ds18b20_is_present(void) {
    return onewire_presence_pulse();
}

float ds18b20_read_temperature(void) {
    if (!onewire_presence_pulse()) {
        ESP_LOGE(DS18B20_TAG, "No DS18B20 sensor present");
        return -999.0; // Error value
    }
    
    onewire_reset();
    onewire_write_byte(0xCC); // Skip ROM command
    onewire_write_byte(0x44); // Convert temperature command
    
    // Wait for conversion (max 750ms for 12-bit resolution)
    vTaskDelay(750 / portTICK_PERIOD_MS);
    
    onewire_reset();
    onewire_write_byte(0xCC); // Skip ROM command
    onewire_write_byte(0xBE); // Read Scratchpad command
    
    // Read 9 bytes from scratchpad
    uint8_t data[9];
    for (int i = 0; i < 9; i++) {
        data[i] = onewire_read_byte();
    }
    
    // Calculate CRC8 checksum
    uint8_t crc = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t inbyte = data[i];
        for (int j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    
    if (crc != data[8]) {
        ESP_LOGW(DS18B20_TAG, "CRC mismatch - temperature reading may be invalid");
    }
    
    // Calculate temperature from raw data
    int16_t temp_raw = (data[1] << 8) | data[0];
    float temperature = temp_raw / 16.0;
    
    ESP_LOGD(DS18B20_TAG, "Raw temperature data: 0x%04X, Temperature: %.2f°C", temp_raw, temperature);
    
    return temperature;
}