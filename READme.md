# Mexendo com Wi-Fi no Raspberry Pi Pico W

Neste projeto, você aprenderá a configurar a conexão Wi-Fi no Raspberry Pi Pico W e a enviar dados para um servidor local. O objetivo é simples: fazer com que a placa envie uma contagem em tempo real para o servidor. Com alguns ajustes, você também pode adaptar o projeto para enviar os dados a um servidor remoto — essa parte fica por sua conta!

## Tecnologias Utilizadas
- **Hardware**: Raspberry Pi Pico W
- **Software**:
    - [Pico C/C++ SDK](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads/13-2-rel1) (SDK)
    - CMake + [GCC](https://jmeubank.github.io/tdm-gcc/) (Compilação)
    - [Node.js](https://nodejs.org/en) + [Express](https://expressjs.com/pt-br/) (Servidor)


# Fazendo o clone
Caso queira apenas um setup inicial, faça o clone deste repositório.

 Para não precisar fazer o download de arquivos desnecessários, você pode utilizar o site  [Download GitHub directory](https://download-directory.github.io) e colocar o link abaixo para baixar apenas a pasta onde estão os arquivos em questão:
```bash
https://github.com/alexsousadev/wifi-init/tree/main/src/wifi-test
```
# Explicação
- ### [1) Preparação do Ambiente](./tutorial/1-setup-inicial/setup-inicial.md)
- ### [2) Configuração do Servidor](./tutorial/2-configuracao-servidor/configurando-servidor.md)
- ### [3) Conexão Wi-Fi no Pico W](./tutorial/3-conexao-wifi/conexao-wifi.md)
- ### [4) Configuração da Conexão TCP/IP](./tutorial/4-conexao-tcp/conexao-tcp.md)

# Conclusão
Espero que este tutorial tenha sido útil para você aprender a configurar o Wi-Fi no Raspberry Pi Pico W e a enviar dados para um servidor. Sinta-se à vontade para implementar suas próprias melhorias, como organizar o código com structs, aumentar a modularidade etc.
