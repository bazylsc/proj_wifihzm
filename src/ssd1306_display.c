#include "ssd1306_display.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static const char *TAG = "SSD1306";

// Display buffer - each bit represents one pixel
static uint8_t display_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Simple 8x8 font (basic ASCII characters)
static const uint8_t font8x8[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (space)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // '!'
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // '"'
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // '#'
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // '$'
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // '%'
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // '&'
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // '''
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // '('
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // ')'
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // '*'
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // '+'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ','
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // '-'
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // '.'
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // '/'
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // '0'
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // '1'
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // '2'
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // '3'
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // '4'
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // '5'
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // '6'
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // '7'
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // '8'
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // '9'
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // ':'
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ';'
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // '<'
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // '='
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // '>'
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // '?'
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // '@'
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // 'A'
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // 'B'
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // 'C'
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // 'D'
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // 'E'
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // 'F'
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // 'G'
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // 'H'
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'I'
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // 'J'
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // 'K'
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // 'L'
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // 'M'
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // 'N'
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // 'O'
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // 'P'
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // 'Q'
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // 'R'
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // 'S'
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // 'T'
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // 'U'
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // 'V'
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // 'W'
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // 'X'
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // 'Y'
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // 'Z'
};

// SSD1306 command definitions
#define SSD1306_SET_CONTRAST_CTRL               0x81
#define SSD1306_SET_ENTIRE_DISPLAY_ON_RESUME    0xA4
#define SSD1306_SET_ENTIRE_DISPLAY_ON            0xA5
#define SSD1306_SET_NORMAL_DISPLAY               0xA6
#define SSD1306_SET_INVERSE_DISPLAY              0xA7
#define SSD1306_SET_DISPLAY_OFF                  0xAE
#define SSD1306_SET_DISPLAY_ON                   0xAF
#define SSD1306_SET_DISPLAY_OFFSET               0xD3
#define SSD1306_SET_COM_PINS_HW_CFG              0xDA
#define SSD1306_SET_VCOM_DESELECT                0xDB
#define SSD1306_SET_DISPLAY_CLK_DIV              0xD5
#define SSD1306_SET_PRECHARGE                    0xD9
#define SSD1306_SET_MULTIPLEX_RATIO              0xA8
#define SSD1306_SET_LOW_COLUMN                   0x00
#define SSD1306_SET_HIGH_COLUMN                  0x10
#define SSD1306_SET_START_LINE                   0x40
#define SSD1306_SET_MEMORY_ADDR_MODE             0x20
#define SSD1306_SET_COLUMN_RANGE                 0x21
#define SSD1306_SET_PAGE_RANGE                   0x22
#define SSD1306_SET_COM_SCAN_DIR                 0xC0
#define SSD1306_SET_COM_SCAN_DIR_OP              0xC8
#define SSD1306_SET_SEGMENT_REMAP                0xA0
#define SSD1306_SET_SEGMENT_REMAP_OP             0xA1
#define SSD1306_SET_CHARGE_PUMP                  0x8D
#define SSD1306_EXTERNAL_VCC                     0x1
#define SSD1306_INTERNAL_VCC                     0x2

// I2C driver handle
static i2c_port_t i2c_num = I2C_NUM_0;

// Send command to SSD1306
static esp_err_t ssd1306_write_command(uint8_t cmd)
{
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SSD1306_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x80, true); // Command mode
    i2c_master_write_byte(cmd_handle, cmd, true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

// Send data to SSD1306
static esp_err_t ssd1306_write_data(uint8_t* data, size_t data_len)
{
    i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
    i2c_master_start(cmd_handle);
    i2c_master_write_byte(cmd_handle, (SSD1306_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd_handle, 0x40, true); // Data mode
    i2c_master_write(cmd_handle, data, data_len, true);
    i2c_master_stop(cmd_handle);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd_handle, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd_handle);
    return ret;
}

// Initialize I2C
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SSD1306_SDA_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = SSD1306_SCL_GPIO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = SSD1306_I2C_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(i2c_num, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(i2c_num, conf.mode, 0, 0, 0);
}

// Initialize SSD1306 display
esp_err_t ssd1306_init(void)
{
    esp_err_t ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "I2C initialized successfully");

    // SSD1306 initialization sequence
    ssd1306_write_command(SSD1306_SET_DISPLAY_OFF);
    ssd1306_write_command(SSD1306_SET_DISPLAY_CLK_DIV);
    ssd1306_write_command(0x80);
    ssd1306_write_command(SSD1306_SET_MULTIPLEX_RATIO);
    ssd1306_write_command(0x3F);
    ssd1306_write_command(SSD1306_SET_DISPLAY_OFFSET);
    ssd1306_write_command(0x00);
    ssd1306_write_command(SSD1306_SET_START_LINE | 0x00);
    ssd1306_write_command(SSD1306_SET_CHARGE_PUMP);
    ssd1306_write_command(0x14);
    ssd1306_write_command(SSD1306_SET_MEMORY_ADDR_MODE);
    ssd1306_write_command(0x00);
    ssd1306_write_command(SSD1306_SET_SEGMENT_REMAP_OP);
    ssd1306_write_command(SSD1306_SET_COM_SCAN_DIR_OP);
    ssd1306_write_command(SSD1306_SET_COM_PINS_HW_CFG);
    ssd1306_write_command(0x12);
    ssd1306_write_command(SSD1306_SET_CONTRAST_CTRL);
    ssd1306_write_command(0xCF);
    ssd1306_write_command(SSD1306_SET_PRECHARGE);
    ssd1306_write_command(0xF1);
    ssd1306_write_command(SSD1306_SET_VCOM_DESELECT);
    ssd1306_write_command(0x40);
    ssd1306_write_command(SSD1306_SET_ENTIRE_DISPLAY_ON_RESUME);
    ssd1306_write_command(SSD1306_SET_NORMAL_DISPLAY);
    ssd1306_write_command(SSD1306_SET_DISPLAY_ON);

    // Clear display buffer
    ssd1306_clear_display();
    ssd1306_display();

    ESP_LOGI(TAG, "SSD1306 initialized successfully");
    return ESP_OK;
}

// Clear display buffer
void ssd1306_clear_display(void)
{
    memset(display_buffer, 0, sizeof(display_buffer));
}

// Update display with buffer content
void ssd1306_display(void)
{
    ssd1306_write_command(SSD1306_SET_COLUMN_RANGE);
    ssd1306_write_command(0);
    ssd1306_write_command(127);
    ssd1306_write_command(SSD1306_SET_PAGE_RANGE);
    ssd1306_write_command(0);
    ssd1306_write_command(7);
    ssd1306_write_data(display_buffer, sizeof(display_buffer));
}

// Set pixel in display buffer
void ssd1306_set_pixel(int x, int y, int color)
{
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT) {
        return;
    }
    
    int byte_idx = x + (y / 8) * SSD1306_WIDTH;
    int bit_idx = y % 8;
    
    if (color) {
        display_buffer[byte_idx] |= (1 << bit_idx);
    } else {
        display_buffer[byte_idx] &= ~(1 << bit_idx);
    }
}

// Write text to display buffer
void ssd1306_write_text(int x, int y, const char* text)
{
    int text_x = x;
    int text_y = y;
    
    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        
        // Handle newline
        if (c == '\n') {
            text_x = x;
            text_y += 8;
            continue;
        }
        
        // Check bounds
        if (text_x + 8 > SSD1306_WIDTH) {
            text_x = x;
            text_y += 8;
        }
        if (text_y + 8 > SSD1306_HEIGHT) {
            break;
        }
        
        // Get character font data (ASCII 32-127)
        if (c >= 32 && c <= 127) {
            const uint8_t* font_data = font8x8[c - 32];
            
            // Draw character
            for (int row = 0; row < 8; row++) {
                uint8_t font_row = font_data[row];
                for (int col = 0; col < 8; col++) {
                    if (font_row & (1 << col)) {
                        ssd1306_set_pixel(text_x + col, text_y + row, 1);
                    }
                }
            }
        }
        
        text_x += 8;
    }
}

// LCD display task - runs in separate thread
static void lcd_display_task(void *parameter)
{
    int counter = 0;
    int update_counter = 0;
    
    ESP_LOGI(TAG, "LCD display task started");
    
    // Initialize SSD1306
    if (ssd1306_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SSD1306");
        vTaskDelete(NULL);
        return;
    }
    
    while (1) {
        // Clear display
        ssd1306_clear_display();
        
        // Get current time
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        char time_str[32];
        char counter_str[32];
        char status_str[32];
        char update_str[32];
        
        // Format time string
        strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
        
        // Format other strings
        snprintf(counter_str, sizeof(counter_str), "Count: %d", counter);
        snprintf(status_str, sizeof(status_str), "Status: RUNNING");
        snprintf(update_str, sizeof(update_str), "Updates: %d", update_counter);
        
        // Write text to display
        ssd1306_write_text(0, 0, "ESP32 LCD Demo");
        ssd1306_write_text(0, 16, time_str);
        ssd1306_write_text(0, 32, counter_str);
        ssd1306_write_text(0, 48, status_str);
        
        // Update display
        ssd1306_display();
        
        // Increment counters
        counter++;
        update_counter++;
        
        ESP_LOGI(TAG, "LCD updated - Counter: %d, Updates: %d", counter, update_counter);
        
        // Wait 2 seconds before next update
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

// Start LCD display task
void start_lcd_display_task(void)
{
    BaseType_t result = xTaskCreate(
        lcd_display_task,       // Function that implements the task
        "LCD_Display_Task",     // Text name for the task
        4096,                   // Stack size in words
        NULL,                   // Parameter passed to the task
        5,                      // Priority (0-25, higher = more important)
        NULL                    // Task handle
    );
    
    if (result == pdPASS) {
        ESP_LOGI(TAG, "LCD display task created successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create LCD display task");
    }
}