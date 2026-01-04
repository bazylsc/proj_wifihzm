#ifndef TELEPLOT_UDP_H
#define TELEPLOT_UDP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Uruchamia wątek wysyłający dane UDP do programu Teleplot
 * 
 * Funkcja tworzy nowy wątek, który co 100ms wysyła przykładowe dane
 * do programu Teleplot przez UDP. Dane zawierają:
 * - Funkcje sinusoidalne (sinus, cosinus)
 * - Dane losowe
 * - Symulację temperatury
 * - Licznik danych
 * 
 * Przed wywołaniem tej funkcji upewnij się, że WiFi jest połączone.
 * 
 * Konfiguracja:
 * - Zmień TELEPLOT_IP w teleplot_udp.c na IP Twojego komputera
 * - Domyślny port: 47269
 */
void start_teleplot_udp_task(void);

#ifdef __cplusplus
}
#endif

#endif // TELEPLOT_UDP_H