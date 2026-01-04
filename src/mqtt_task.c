#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "cJSON.h"

static const char *TAG = "MQTT_TASK";

/* MQTT Configuration */
#define MQTT_BROKER_URI "mqtt://broker.emqx.io"  // Przykładowy publiczny broker
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "esp32_client"
#define MQTT_SUBSCRIBE_TOPIC "esp32/data"
#define MQTT_PUBLISH_TOPIC "esp32/status"

/* MQTT Event Group */
static EventGroupHandle_t mqtt_event_group;
const int MQTT_CONNECTED_BIT = BIT0;

/* MQTT Client Handle */
static esp_mqtt_client_handle_t client;

/* WiFi credentials - należy dostosować */
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

/* WiFi Event Group */
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

/* WiFi Event Handler */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "retry to connect to the AP");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* WiFi Initialization */
void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

/* MQTT Event Handler */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        
        // Subskrypcja na temat
        msg_id = esp_mqtt_client_subscribe(client, MQTT_SUBSCRIBE_TOPIC, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        
        // Publikacja wiadomości powitalnej
        msg_id = esp_mqtt_client_publish(client, MQTT_PUBLISH_TOPIC, "ESP32 Connected", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
        
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        
        // Obsługa otrzymanych danych
        mqtt_handle_received_data(event->topic, event->topic_len, event->data, event->data_len);
        break;
        
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
        
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

/* MQTT Client Initialization */
void mqtt_app_start(void)
{
    mqtt_event_group = xEventGroupCreate();
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .broker.address.port = MQTT_PORT,
        .credentials.client_id = MQTT_CLIENT_ID,
        .credentials.username = NULL,  // Ustaw jeśli broker wymaga autentykacji
        .credentials.authentication.password = NULL,  // Ustaw jeśli broker wymaga autentykacji
        .session.keepalive = 60,
        .session.disable_clean_session = 0,
        .network.reconnect_timeout_ms = 5000,
        .network.timeout_ms = 5000,
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client) {
        esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
        esp_mqtt_client_start(client);
        ESP_LOGI(TAG, "MQTT client started");
    } else {
        ESP_LOGE(TAG, "Failed to initialize MQTT client");
    }
}

/* Handle received MQTT data */
void mqtt_handle_received_data(const char* topic, int topic_len, const char* data, int data_len)
{
    char topic_str[topic_len + 1];
    char data_str[data_len + 1];
    
    strncpy(topic_str, topic, topic_len);
    topic_str[topic_len] = '\0';
    
    strncpy(data_str, data, data_len);
    data_str[data_len] = '\0';
    
    ESP_LOGI(TAG, "Received topic: %s, data: %s", topic_str, data_str);
    
    // Próba parsowania JSON
    cJSON *json = cJSON_Parse(data_str);
    if (json != NULL) {
        cJSON *command = cJSON_GetObjectItem(json, "command");
        if (cJSON_IsString(command)) {
            ESP_LOGI(TAG, "Command: %s", command->valuestring);
            
            if (strcmp(command->valuestring, "led_on") == 0) {
                // Tutaj można dodać kod do włączenia LED
                ESP_LOGI(TAG, "LED ON command received");
                mqtt_publish_status("LED turned ON");
            }
            else if (strcmp(command->valuestring, "led_off") == 0) {
                // Tutaj można dodać kod do wyłączenia LED
                ESP_LOGI(TAG, "LED OFF command received");
                mqtt_publish_status("LED turned OFF");
            }
            else if (strcmp(command->valuestring, "status") == 0) {
                mqtt_publish_device_status();
            }
        }
        cJSON_Delete(json);
    }
}

/* Publish status message */
void mqtt_publish_status(const char* message)
{
    if (client && (xEventGroupGetBits(mqtt_event_group) & MQTT_CONNECTED_BIT)) {
        int msg_id = esp_mqtt_client_publish(client, MQTT_PUBLISH_TOPIC, message, 0, 1, 0);
        ESP_LOGI(TAG, "Status published, msg_id=%d", msg_id);
    } else {
        ESP_LOGW(TAG, "MQTT not connected, cannot publish status");
    }
}

/* Publish device status as JSON */
void mqtt_publish_device_status(void)
{
    cJSON *status_json = cJSON_CreateObject();
    cJSON *device_id = cJSON_CreateString("ESP32_001");
    cJSON *uptime = cJSON_CreateNumber(esp_timer_get_time() / 1000000); // uptime w sekundach
    cJSON *free_heap = cJSON_CreateNumber(esp_get_free_heap_size());
    cJSON *timestamp = cJSON_CreateNumber(esp_timer_get_time());
    
    cJSON_AddItemToObject(status_json, "device_id", device_id);
    cJSON_AddItemToObject(status_json, "uptime", uptime);
    cJSON_AddItemToObject(status_json, "free_heap", free_heap);
    cJSON_AddItemToObject(status_json, "timestamp", timestamp);
    
    char *json_string = cJSON_Print(status_json);
    if (json_string) {
        mqtt_publish_status(json_string);
        free(json_string);
    }
    
    cJSON_Delete(status_json);
}

/* MQTT Task */
void mqtt_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Starting MQTT task");
    
    // Oczekiwanie na połączenie WiFi
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "WiFi connected, starting MQTT client");
    
    // Uruchomienie klienta MQTT
    mqtt_app_start();
    
    // Oczekiwanie na połączenie MQTT
    xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "MQTT connected");
    
    // Główna pętla zadania
    while (1) {
        if (xEventGroupGetBits(mqtt_event_group) & MQTT_CONNECTED_BIT) {
            // Publikuj status co 30 sekund
            mqtt_publish_device_status();
            vTaskDelay(pdMS_TO_TICKS(30000));
        } else {
            ESP_LOGW(TAG, "MQTT disconnected, waiting for reconnection...");
            xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, false, true, portMAX_DELAY);
        }
    }
}

/* Initialize MQTT task */
void mqtt_task_init(void)
{
    ESP_LOGI(TAG, "Initializing MQTT task");
    
    // Inicjalizacja WiFi
    wifi_init_sta();
    
    // Utworzenie zadania MQTT
    xTaskCreate(mqtt_task, "mqtt_task", 4096 * 2, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "MQTT task created");
}