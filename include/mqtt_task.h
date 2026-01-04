#ifndef MQTT_TASK_H
#define MQTT_TASK_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize WiFi station mode
 */
void wifi_init_sta(void);

/**
 * @brief Start MQTT application
 */
void mqtt_app_start(void);

/**
 * @brief Handle received MQTT data
 * 
 * @param topic MQTT topic
 * @param topic_len Topic length
 * @param data Message data
 * @param data_len Data length
 */
void mqtt_handle_received_data(const char* topic, int topic_len, const char* data, int data_len);

/**
 * @brief Publish status message
 * 
 * @param message Status message to publish
 */
void mqtt_publish_status(const char* message);

/**
 * @brief Publish device status as JSON
 */
void mqtt_publish_device_status(void);

/**
 * @brief Main MQTT task function
 * 
 * @param pvParameters Task parameters
 */
void mqtt_task(void *pvParameters);

/**
 * @brief Initialize MQTT task
 */
void mqtt_task_init(void);

#ifdef __cplusplus
}
#endif

#endif /* MQTT_TASK_H */