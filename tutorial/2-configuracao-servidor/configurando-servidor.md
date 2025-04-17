
## 2) Configurando nosso servidor
Precisamos criar nosso servidor local para receber os envios de dados. Nesse caso, usarei o [Express](https://expressjs.com/) com Typescript, que é um framework [Node.js](https://nodejs.org/en). Mas você pode usar qualquer outro, **a importância aqui é criar a rota para receber os dados.**

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
### Ligando o Servidor

Para ligar o servidor, basta instalar as dependências (entre na raiz do server primeiro, na pasta `📁server`) com o comando:
```bash
npm i
```
E então, iniciar o servidor:
```
npm run start
```
---
- ### Próximo: [Configuração do Wi-Fi](../3-conexao-wifi/conexao-wifi.md)