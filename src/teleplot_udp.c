#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

// Konfiguracja dla Teleplot
#define TELEPLOT_IP     "192.168.5.45"  // Zmień na IP komputera z teleplot
#define TELEPLOT_PORT   47269             // Domyślny port teleplot
#define UDP_BUFFER_SIZE 256

static const char *UDP_TAG = "teleplot_udp";

// Struktura do przechowywania danych UDP
typedef struct {
    int socket_fd;
    struct sockaddr_in dest_addr;
} udp_context_t;

// Funkcja inicjalizująca połączenie UDP
static int init_udp_connection(udp_context_t *ctx) {
    // Utworzenie socketu UDP
    ctx->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (ctx->socket_fd < 0) {
        ESP_LOGE(UDP_TAG, "Nie można utworzyć socketu: errno %d", errno);
        return -1;
    }

    // Konfiguracja adresu docelowego
    ctx->dest_addr.sin_addr.s_addr = inet_addr(TELEPLOT_IP);
    ctx->dest_addr.sin_family = AF_INET;
    ctx->dest_addr.sin_port = htons(TELEPLOT_PORT);

    ESP_LOGI(UDP_TAG, "UDP socket utworzony, wysyłanie do %s:%d", TELEPLOT_IP, TELEPLOT_PORT);
    return 0;
}

// Funkcja wysyłająca dane do teleplot
static void send_teleplot_data(udp_context_t *ctx, const char *name, float value) {
    char buffer[UDP_BUFFER_SIZE];
    
    // Format danych dla teleplot: ">nazwa:wartość\n"
    int len = snprintf(buffer, sizeof(buffer), ">%s:%.3f\n", name, value);
    
    if (len > 0 && len < sizeof(buffer)) {
        int err = sendto(ctx->socket_fd, buffer, len, 0, 
                        (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));
        if (err < 0) {
            ESP_LOGW(UDP_TAG, "Błąd wysyłania danych: errno %d", errno);
        }
    }
}

// Główna funkcja wątku UDP
void teleplot_udp_task(void *pvParameters) {
    udp_context_t udp_ctx;
    
    ESP_LOGI(UDP_TAG, "Uruchamianie wątku Teleplot UDP...");
    
    // Poczekaj aż WiFi się połączy (może być potrzebne)
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // Inicjalizacja połączenia UDP
    if (init_udp_connection(&udp_ctx) != 0) {
        ESP_LOGE(UDP_TAG, "Inicjalizacja UDP nie powiodła się");
        vTaskDelete(NULL);
        return;
    }
    
    float time_counter = 0.0;
    int data_counter = 0;
    
    while (1) {
        // Generowanie przykładowych danych do wizualizacji
        float sine_wave = sin(time_counter * 0.1) * 100.0;
        float cosine_wave = cos(time_counter * 0.15) * 50.0;
        float random_data = (rand() % 100) - 50;
        float temperature_sim = 25.0 + sin(time_counter * 0.05) * 10.0;
        
        // Wysyłanie danych do teleplot
        send_teleplot_daUDP
ta(&udp_ctx, "sinus", sine_wave);
        send_teleplot_data(&udp_ctx, "cosinus", cosine_wave);
        send_teleplot_data(&udp_ctx, "random", random_data);
        send_teleplot_data(&udp_ctx, "temp", temperature_sim);
        send_teleplot_data(&udp_ctx, "counter", (float)data_counter);
        
        // Informacja o wysłanych danych co 50 iteracji
        if (data_counter % 50 == 0) {
            ESP_LOGI(UDP_TAG, "Wysłano dane #%d - sinus: %.2f, temp: %.2f°C", 
                     data_counter, sine_wave, temperature_sim);
        }
        
        time_counter += 1.0;
        data_counter++;
        
        // Wysyłaj dane co 100ms
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Zamknij socket przed zakończeniem (nigdy nie powinno się wykonać)
    close(udp_ctx.socket_fd);
    vTaskDelete(NULL);
}

// Funkcja do uruchomienia wątku teleplot
void start_teleplot_udp_task(void) {
    xTaskCreate(
        teleplot_udp_task,      // Funkcja wątku
        "TeleplotUDP",          // Nazwa wątku
        4096,                   // Rozmiar stosu w bajtach
        NULL,                   // Parametry
        5,                      // Priorytet
        NULL                    // Handle wątku
    );
    ESP_LOGI(UDP_TAG, "Wątek Teleplot UDP został utworzony");
}