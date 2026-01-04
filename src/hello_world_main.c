/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

// Funkcja dla dodatkowego wątku
void additional_task(void *parameter) {
    int counter = 0;
    
    while(1) {
        printf("[Dodatkowy wątek] Licznik: %d\n", counter++);
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Opóźnienie 2 sekundy
    }
}

void app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);

    // Tworzenie dodatkowego wątku
    xTaskCreate(
        additional_task,        // Funkcja wątku
        "AdditionalTask",       // Nazwa wątku
        2048,                   // Rozmiar stosu w słowach
        NULL,                   // Parametr przekazywany do wątku
        5,                      // Priorytet (0-25, wyższe = ważniejsze)
        NULL                    // Handle do wątku (opcjonalne)
    );

    printf("Dodatkowy wątek został utworzony!\n");

    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%uMB %s flash\n", flash_size / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
