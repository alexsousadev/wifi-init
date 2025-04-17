#include "send-data.h"

#define SERVER_IP "192.x.x.x"  // Troque por seu ip
#define SERVER_PORT 3000       // Troque por sua porta
#define SERVER_PATH "/receber" // Troque por sua rota

// Callback para quando os dados são enviados
static err_t sent_callback(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    printf("Dados enviados com sucesso!\n");
    tcp_close(pcb); // Fecha a conexão TCP
    return ERR_OK;
}

// Envia dados para o servidor
void send_data_to_server(const char *path, char *request_body, const char *type_method)
{
    // Criando PCB
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("Erro ao criar PCB\n");
        return;
    }

    // Organizando o endereço IP
    ip_addr_t server_ip;
    server_ip.addr = ipaddr_addr(SERVER_IP);

    // Conectando ao servidor
    if (tcp_connect(pcb, &server_ip, SERVER_PORT, NULL) != ERR_OK)
    {
        printf("Erro ao conectar ao servidor\n");
        tcp_abort(pcb);
        return;
    }

    // Montando requisição
    char request[521];
    snprintf(request, sizeof(request),
             "%s %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             type_method, path, SERVER_IP, strlen(request_body), request_body);

    // Definindo o callback para quando os dados forem enviados com sucesso
    tcp_sent(pcb, sent_callback);

    // Empacotando a requisição
    if (tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY) != ERR_OK)
    {
        printf("Erro ao enviar dados\n");
        tcp_abort(pcb);
        return;
    }

    // Enviando a requisição
    if (tcp_output(pcb) != ERR_OK)
    {
        printf("Erro ao enviar dados (tcp_output)\n");
        tcp_abort(pcb);
        return;
    }
}

// Criando uma requisição
void create_request(int data)
{
    const char *type_method = "POST";
    const char *path = SERVER_PATH;
    char json_request[256];

    // Preparando o corpo da requisição
    snprintf(json_request, sizeof(json_request),
             "{ \"dado\" : %d }",
             data);

    // Enviando requisição para o servidor
    send_data_to_server(path, json_request, type_method);
}
