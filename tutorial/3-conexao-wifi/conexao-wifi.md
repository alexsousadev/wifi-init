
## 3) Configurando a conexão Wi-Fi no Pico W
Agora sim podemos começar de fato a mexer no código do Raspberry Pi Pico W. Primeiro, precisamos importar as bibliotecas necessárias:
### 3.1 Importando as bibliotecas

```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"    
#include "lwip/ip_addr.h" 
```
A `stdio.h`e `pico/stdlib.h` já são conhecidas, então vamos falar do resto:
- `pico/cyw43_arch.h`: Biblioteca que inicializa e controla o chip Wi-Fi (CYW43) do Pico W
- `lwip/tcp.h`: Gerencia toda a parte de conexão TCP
    > O TCP é o protocolo responsável por fazer a conexão entre o cliente (nossa plaquinha) e o servidor.
- `lwip/ip_addr.h`: Lida com endereços IP

Usaremos elas em:
| Biblioteca                    | Usos / Métodos Associados                          |
|------------------------------|-----------------------------------------------------|
| `#include "pico/cyw43_arch.h"` | `cyw43_arch_init()`, `cyw43_arch_enable_sta_mode()`, `cyw43_arch_wifi_connect_timeout_ms()`             |
| `#include "lwip/tcp.h"`      | `tcp_new()`, `tcp_connect()`, `tcp_write()`, `tcp_output()` |
| `#include "lwip/ip_addr.h"`  | `ip_addr_t`, `ipaddr_addr()`                      |


### 3.2 Inicializando a conexão Wi-Fi
Primeiro, vamos inicializar variáveis com o nome da rede e senha:
```c
#define WIFI_SSID "PICOWIFI" // Troque pelo nome da sua rede
#define WIFI_PASSWORD "12345678" // Troque pela senha da sua rede
```
Agora, vamos usar os métodos da biblioteca `pico/cyw43_arch.h` para inicializar a conexão Wi-Fi:

```c
void init_wifi()
{
    // Inicialização do Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return;
    }

    // Ativação do modo STA (cliente)
    cyw43_arch_enable_sta_mode();
    
    // Conectando ao Wi-Fi com timeout de 10 segundos
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Erro ao conectar ao Wi-Fi: tentando novamente...\n");
        sleep_ms(3000);
    }

    printf("Wi-Fi conectado com sucesso!\n");
}
```

- `cyw43_arch_init()`: Inicia o chip Wi-Fi interno do Pico W
- `cyw43_arch_enable_sta_mode()`: Coloca o Wi-Fi no modo cliente (STA = Station), ou seja, o Pico vai entrar em uma rede, não criar uma
- `cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)`: Tenta se conectar à rede Wi-Fi com:
    - `WIFI_SSID`: Nome da rede Wi-Fi
    - `WIFI_PASSWORD`: Senha da rede Wi-Fi
    - `CYW43_AUTH_WPA2_AES_PSK`: Método de autenticação (WPA2-PSK = Protected Wi-Fi). Pode ver mais sobre isso [aqui](https://tecnoblog.net/responde/o-que-e-wep-wpa-wpa2-wpa3-diferencas-protocolo-seguranca-wi-fi/)
    - `10000`: Tempo de timeout (Basicamente, vai tentar conectar ao Wi-Fi por 10 segundos, se não der certo, avisa que falhou)”
    > O while serve para ficar em loop enquanto a conexão não for estabelecida.
---
### 3.3 Organizando a conexão e os dados
Agora que já temos a conexão com a internet, precisamos definir para onde vamos conectar. Para isso, vamos definir as variáveis para o endereço IP, porta e caminho do local onde vamos enviar os dados (nesse caso, é tudo do nosso PC Local):
```c
#define SERVER_IP "192.x.x.x"    // Troque por seu ip 
#define SERVER_PORT 3000        // Troque por sua porta
#define SERVER_PATH "/receber" // Troque por sua rota
```

Depois, vamos organizar a estrutura da requisição. Primeiro, formatando o JSON que será enviado:
```c
    const char *type_method = "POST";
    const char *path = SERVER_PATH;
    char json_request[256];

    // Preparando o corpo (body) da requisição
    snprintf(json_request, sizeof(json_request),
             "{ \"dado\" : %d }",
             data);
}
```
- Usamos o `snprintf` para formatar o corpo da requisição (Veja mais sobre esse método [aqui](https://www.tutorialspoint.com/c_standard_library/c_function_sprintf.htm)). Na prática, estamos organizando assim:
    ```c
    { "dado" : <dado> }
    ```
    > Nesse caso, o dado será um `decimal` (%d), mas você pode usar qualquer outro tipo de dado, desde que faça os ajustes necessários.

Por enquanto, temos o `body`, mas precisamos de um `header` também. É o que vamos fazer agora! (Caso esteja perdido, pesquise um pouco sobre a [estrutura de uma requisição HTTP](https://mazer.dev/pt-br/http/introducao-protocolo-http/)


Para relembrar, precisamos de:
- `method`: Método de requisição (o `type_method`)
- `path`: Caminho da requisição (a `SERVER_PATH`)
- `body`: Corpo da requisição (a `json_request`)
- `header`: Informações adicionais sobre a requisição (vamos fazer agora)

já temos tudo, então vamos organizar:
```
   char request[512];
   snprintf(request, sizeof(request),
             "%s %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             type_method, path, SERVER_IP, strlen(request_body), request_body);
```
 A função `snprintf` é usada novamente para formatar a requisição. Para ficar mais visual, estamos organizando assim:
```
        POST /receber HTTP/1.1
        Host: 192.168.0.107
        Content-Type: application/json
        Content-Length: 16

        { "dado" : <dado> }
```
> O header é essa parte do Host, Content-Type e Content-Length que você pode ver na requisição HTTP.

---
- ### Próximo: [Configuração da conexão TCP/IP](../4-conexao-tcp/conexao-tcp.md)