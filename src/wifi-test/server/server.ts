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