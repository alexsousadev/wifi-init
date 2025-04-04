#include "lwip/tcp.h"     // blioteca que gerencia conexões TCP/IP
#include "lwip/ip_addr.h" // Lida com endereços IP.
#include "pico/cyw43_arch.h"

void send_data_to_server(const char *path, char *request_body, const char *type_method);
void create_request(int data);