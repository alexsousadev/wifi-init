#include "wifi-connect.h"

// Inicialização do wifi e conectando na rede
void init_wifi()
{
    // Inicialização do Wi-Fi
    if (cyw43_arch_init())
    {
        printf("Erro ao inicializar Wi-Fi\n");
        return;
    }

    // Ativação do modo STA (cliente)
    cyw43_arch_enable_sta_mode();

    // Conectando ao Wi-Fi com timeout de 10 segundos
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("Erro ao conectar ao Wi-Fi: tentando novamente...\n");
        sleep_ms(3000);
    }

    printf("Wi-Fi conectado com sucesso!\n");
}
