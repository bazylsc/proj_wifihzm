#ifndef SSD1306_DISPLAY_H
#define SSD1306_DISPLAY_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

// SSD1306 I2C parameters
#define SSD1306_I2C_ADDRESS     0x3C
#define SSD1306_SDA_GPIO        20 // GPIO_NUM_21
#define SSD1306_SCL_GPIO        21 //GPIO_NUM_22
#define SSD1306_I2C_FREQ_HZ     400000

// Display dimensions
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

// Function declarations
esp_err_t ssd1306_init(void);
void ssd1306_clear_display(void);
void ssd1306_display(void);
void ssd1306_write_text(int x, int y, const char* text);
void ssd1306_set_pixel(int x, int y, int color);
void start_lcd_display_task(void);

#ifdef __cplusplus
}
#endif

#endif // SSD1306_DISPLAY_H