const net = require('net');
const express = require('express');
const path = require('path');

let ultimoDado = "Aguardando dados da placa...";

// Conexão TCP com a Pico W
const tcpClient = new net.Socket();
tcpClient.connect(80, '10.9.131.87', () => {
    console.log('Conectado à Raspberry Pi Pico W');
});

tcpClient.on('data', (data) => {
    ultimoDado = data.toString();
    console.log('Recebido da Pico:', ultimoDado);
});

tcpClient.on('error', (err) => {
    console.log('Erro TCP:', err.message);
});

const app = express();
const PORT = 3000;

app.use(express.static('public'));

app.get('/dados', (req, res) => {
    res.send(ultimoDado);
});

app.listen(PORT, () => {
    console.log(`Servidor HTTP rodando em http://localhost:${PORT}`);
});
