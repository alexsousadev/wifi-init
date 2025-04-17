

## 4) Configurando a conexão TCP/IP

### 4.1 Estabelecendo a conexão
Ok, ja reúnimos todos o dados necessários para enviar a requisição. Agora, vamos criar a conexão com o servidor. Para isso, vamos criar um novo canal TCP:

```c
struct tcp_pcb *pcb = tcp_new();
if (!pcb) {
    printf("Erro ao criar PCB\n");
    return;
}
```
- `tcp_pcb`: Essa struct representa o bloco de controle de protocolo (Protocol Control Block) para uma conexão TCP. Ela é um tipo que vem da biblioteca `lwip/tcp.h`

Precisamos transformar o endereço IP que está em string para um formato que o a lwip entende, então iremos definir com o tipo `ip_addr_t` e usamos a função `ipaddr_addr()` para a conversão:

```c
ip_addr_t server_ip;
server_ip.addr = ipaddr_addr(SERVER_IP);
```
Agora que temos o ip legível, podemos estabelecer a conexão TCP. Para resumir, precisamos de:
- `Protocol Control Block (PCB)`: Bloco de controle TCP
    > É uma estrutura de dados que mantém as informações necessárias para gerenciar a conexão TCP
- `IP`: Endereço IP do servidor (Nosso PC local)
- `Porta`: Porta do servidor
- `Callback`: Uma função que será executada quando a conexão TCP for estabelecida com sucesso. Nesse caso, podemos colocar NULL, pois não vamos usar nenhum callback.

    ```c
    // Conectando ao servidor
        if (tcp_connect(pcb, &server_ip, SERVER_PORT, NULL) != ERR_OK)
        {
            printf("Erro ao conectar ao servidor\n");
            tcp_abort(pcb);
            return;
        }
    ```
    Segundo a documentação oficial do **LwIp**, o `tcp_connect` retorna `ERR_VAL` se forem fornecidos argumentos inválidos e `ERR_OK` se a solicitação de conexão tiver sido enviada com sucesso. Por isso, estamos verificando se o retorno é `ERR_OK` e caso não seja, abortamos a conexão.

    >Outros valores de erro (`err_t`) podem ser retornados caso a solicitação de conexão não possa ser enviada, como erros relacionados à pilha TCP/IP ou ao estado do PCB. Veja mais [aqui](https://www.nongnu.org/lwip/2_1_x/group__tcp__raw.html#ga9a31deea4cadacd39f9485f37cfdd012)
---
### 4.2 Empacotando os dados 


Agora que já estabelecemos a conexão TCP, vamos empacotar os dados que queremos enviar. Para isso, usamos `tcp_write()`. Mas antes, precisamos criar uma função que será executada quando os dados forem enviados com sucesso. Para isso, vamos usar a função `tcp_sent()`.

```c
tcp_sent(pcb, sent_callback);
```

A nossA funçao `sent_callback` será executada quando os dados forem enviados com sucesso. Ela irá printar uma mensagem que os dados foram enviados com sucesso e fechará a conexão TCP:
```c
static err_t sent_callback(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    printf("Dados enviados com sucesso!\n");
    tcp_close(pcb); // Fecha a conexão TCP
    return ERR_OK;
}
```
> A função `tcp_close(pcb)` serve pra fechar uma conexão TCP que foi usada e liberar os recursos (memória, buffers, ponteiros etc) que estavam sendo usados naquela conexão. Isso evita **vazamento de memória**, que pode travar o sistema. Veja mais [aqui](https://www.datacamp.com/pt/blog/memory-leak)
---
Agora que já temos a função callback que vai lidar com os dados quando forem enviados, podemos começar a empacotar eles:

```c
if (tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY) != ERR_OK)
```
- `pcb`: O objeto de conexão TCP  
- `request`: O que queremos enviar (requisição HTTP formatada).
- `strlen(request)`: o tamanho da mensagem
- `TCP_WRITE_FLAG_COPY`: Diz que o TCP deve copiar os dados, não só apontar para eles. 

### 4.3 Enviando os dados

Depois de empacotar, vamos enviar de fato com `tcp_output()`:
```c
 // Tratando possíveis erros ao enviar requisição
    if (tcp_output(pcb) != ERR_OK)
    {
        printf("Erro ao enviar dados (tcp_output)\n");
        tcp_abort(pcb);
        return;
    }
```
---
### Trecho de Código completo
```c
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

```

---
## 4 Juntando na main
Nesse caso, vamos inicializar a conexão Wi-Fi e então enviar dados para o servidor. Vamos só enviar uma contagem:

```c
int main()
{
    stdio_init_all();
    connect_to_wifi();

    int data = 0;

    while (true)
    {
        printf("Enviando dados para o servidor...\n");

        create_request(data++); // Envia para o servidor (0, 1, 2, 3...)

        sleep_ms(3000); // Espera 3 segundos antes do próximo envio
    }
}
```
---
- ### [Voltar para o início](../../READme.md)