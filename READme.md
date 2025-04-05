# Mexendo com Wi-Fi no Raspberry Pi Pico W

Neste projeto, você vai aprender como configurar a conexão Wi-Fi no Raspberry Pi Pico W e enviar dados para um servidor local. O objetivo é simples: fazer a placa enviar um contagem em tempo real para o servidor. Com alguns ajustes, você também pode adaptar o projeto para enviar os dados para um servidor remoto — mas essa parte deixo por sua conta!


## Sumário
- [1) 🔧 Reunindo o Setup Inicial](#1-reunindo-o-setup-inicial)
    - 1.1 [Pegando o arquivo `lwipopts.h`](#11-pegando-o-arquivo-lwipopts)
   - 1.2 [Obtendo o IP da máquina](#12-pegando-o-ip-da-máquina)  
   - 1.3 [Obtendo informações do Wi-Fi](#13-pegando-informações-do-wi-fi) 
   - 1.4 [Configurando o CMakeLists](#14-configurando-o-cmakelists)  
- [2) 🌐 Configurando nosso servidor](#2-configurando-nosso-servidor)
- [3) 📶 Configurando a conexão no Wi-Fi no Pico W](#3-configurando-a-conexão-wi-fi-no-pico-w)
    - 3.1 [Importando as bibliotecas](#31-importando-as-bibliotecas)
    - 3.2 [Inicializando a conexão Wi-Fi](#32-inicializando-a-conexão-wi-fi)
    - 3.3 [Organizando a conexão e os dados](#33-organizando-a-conexão-e-os-dados)
- [ 4) ⚙️ Configurando a conexão TCP/IP](#4-configurando-a-conexão-tcpip)
    - 4.1 [Estabelecendo a conexão](#41-estabelecendo-a-conexão)
    - 4.2 [Empacotando os dados](#42-empacotando-os-dados)
    - 4.3 [Enviando os dados](#43-enviando-os-dados)
 
# Fazendo clone
Caso queira apenas um setup inicial, faça o clone:
```bash
git clone https://github.com/alexsousadev/wifi-init
```
# Explicação
Caso queira entender a lógica, venha por aqui...
## 1) Reunindo o Setup Inicial

### 1.1 Pegando o arquivo lwipopts
Primeiro, precisamos colocar na raiz do nosso projeto o arquivo `lwipopts.h`. Pegue [aqui](https://github.com/alexsousadev/wifi-init/blob/main/lwipopts.h)

> Este arquivo contém as configurações padrão do lwIP (Lightweight IP), que é uma pilha de protocolos TCP/IP de código aberto, projetada especificamente para sistemas embarcados. Veja mais [aqui](https://www.nongnu.org/lwip/2_1_x/group__tcp__raw.html)

### 1.2) Pegando o IP da máquina
Como estamos enviando a requisição para um servidor local, precisamos saber o endereço IP do servidor (nosso PC). Para isso, abra o terminal e digite o seguinte comando:

- **Linux**: `hostname -I` 
- **Windows**: `ipconfig`

Isso vai retornar seu endereço IP (v4). Geralmente, é o primeiro que aparece na listagem (192.x.x.x)
### 1.3) Pegando informações do Wi-Fi
Para concectar a placa no Wi-Fi, precisamos saber o SSID (nome da rede) e a senha. **Então, tenha isso em mãos.**

### 1.4) Configurando o CMakeLists
No Cmake vamos precisar apenas adicionar uma linha em `target_link_libraries`:

```c
target_link_libraries(wifi-test
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        )

```
Explicação da sigla:
- `pico_cyw43_arch` → É a **arquitetura** que integra o **chip CYW43** (Wi-Fi + LED) no ambiente do **Raspberry Pi Pico**
    > No Pico W, o LED integrado é controlado pelo Wi-Fi
- `lwip` → Significa que ele está usando a **pilha de rede lwIP** (Lightweight IP)
- `threadsafe` → Significa que é **seguro para usar com múltiplas "threads" ou núcleos** (quando roda coisas em paralelo ou callbacks)
- `background` → A rede é cuidada em **background**, ou seja, **você não precisa chamar nada no `main loop`**

A Lib cyw43 tem outros modos que você pode ver [aqui](https://www.raspberrypi.com/documentation/pico-sdk/networking.html#group_pico_cyw43_arch), como o "modo manual" e o "modo LED" (não carrega a pilha TCP/IP e habilita só o controle do LED)


## 2) Configurando nosso servidor
Precisamos criar nosso servidor local para receber os envios de dados. Nesse caso, usarei o [Express](https://expressjs.com/) com Typescript, que é um framework Node.js. Mas você pode usar qualquer outro, **a importância aqui é criar a rota para receber os dados.**

```ts
import express, { Request, Response } from 'express';
const app = express();

const PORT = 3000;

app.use(express.json());

app.post('/receber', (req: Request, res: Response) => {
    const { dado } = req.body;
    console.log('Dado recebido:', dado);
    res.send('Dados recebidos com sucesso!');
});

app.listen(PORT, () => {
    console.log('Servidor rodando em http://localhost:3000');
});
```
Detalhes do que está acontecendo:
- Teremos uma rota `/receber` que receberá os dados enviados pelo cliente (que é a plaquinha).
- O JSON recebido vem no formato:
    ```
    {
        "dado": <valor>
    }
    ```

## 3) Configurando a conexão Wi-Fi no Pico W
Agora sim podemos começar de fato a mexer no código do Raspberry Pi Pico W. Primeiro, precisamos importar as bibliotecas necessárias:
### 3.1) Importando as bibliotecas

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


### 3.2) Inicializando a conexão Wi-Fi
Primeiro, vamos inicializar variáveis com o nome da rede e senha:
```c
#define WIFI_SSID "PICOWIFI"
#define WIFI_PASSWORD "12345678"
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
    - `CYW43_AUTH_WPA2_AES_PSK`: Método de autenticação (WPA2-PSK = Protected Wi-Fi). Pode ver mais sobre isso [aqui]("https://tecnoblog.net/responde/o-que-e-wep-wpa-wpa2-wpa3-diferencas-protocolo-seguranca-wi-fi/")
    - `10000`: Tempo de timeout (Basicamente, vai tentar conectar ao Wi-Fi por 10 segundos, se não der certo, avisa que falhou)”
    > O while serve para ficar em loop enquanto a conexão não for estabelecida.
---
### 3.3) Organizando a conexão e os dados
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
## 4) Configurando a conexão TCP/IP

### 4.1) Estabelecendo a conexão
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
### 4.2) Empacotando os dados 
Agora que já estabelecemos a conexão TCP, vamos empacotar os dados que queremos enviar. Para isso, usamos a função `tcp_write()`.
```c
if (tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY) != ERR_OK)
```
- `request`: O que queremos enviar (requisição HTTP formatada).
- `strlen(request)`: o tamanho da mensagem
- `TCP_WRITE_FLAG_COPY`: Diz que o TCP deve copiar os dados, não só apontar para eles.

### 4.3) Enviando os dados

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
#define SERVER_IP "192.x.x.x"    // Troque pro seu ip 
#define SERVER_PORT 3000        // Troque por sua porta
#define SERVER_PATH "/receber" // Troque por sua rota

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
## 4) Juntando na main
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
# Conclusão
Espero que esse tutorial tenha sido útil para você aprender a configurar o Wi-Fi no Raspberry Pi Pico W e como enviar dados para um servidor. Fique livre para fazer suas própiras melhorias, como práticas melhores (organização com structs, mais modularidade etc) e comentários mais explicativos.
