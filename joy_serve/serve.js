// const express = require('express');
// const net = require('net');

// const app = express();
// const PORT = 3000;
// const TCP_PORT = 80;  // Porta TCP onde o dispositivo C estará enviando os dados

// let sensorData = {};  // Objeto para armazenar os dados do dispositivo C

// // Função para lidar com as conexões TCP
// const handleTCPConnection = (socket) => {
//     console.log('Cliente TCP conectado');

//     // Quando o dispositivo C enviar dados via TCP
//     socket.on('data', (data) => {
//         console.log('Dados recebidos via TCP:', data.toString());
//         try {
//             const jsonData = JSON.parse(data.toString());
//             sensorData = jsonData;  // Atualiza os dados recebidos
//             console.log('Dados atualizados:', jsonData);
//         } catch (err) {
//             console.error('Erro ao processar dados JSON:', err);
//         }
//     });

//     // Tratamento de desconexão
//     socket.on('end', () => {
//         console.log('Cliente TCP desconectado');
//     });

//     // Tratamento de erro
//     socket.on('error', (err) => {
//         console.error('Erro na conexão TCP:', err);
//     });
// };

// // Cria o servidor TCP
// const tcpServer = net.createServer(handleTCPConnection);

// // Inicia o servidor TCP
// tcpServer.listen(TCP_PORT, () => {
//     console.log(`Servidor TCP ouvindo na porta ${TCP_PORT}`);
// });

// // Rota para '/'
// app.get('/', (req, res) => {
//     res.send('Bem-vindo ao servidor!');
// });

// // API RESTful que fornece os dados para o frontend
// app.get('/dados', (req, res) => {
//     if (!sensorData) {
//         console.log('Nenhum dado disponível');
//     }
//     res.json(sensorData);  // Retorna os dados do dispositivo C
// });

// // Inicia o servidor HTTP
// app.listen(PORT, () => {
//     console.log(`Servidor HTTP rodando na porta ${PORT}`);
// });
const express = require('express');
const app = express();
const port = 3000;

// Variáveis para armazenar os dados recebidos da Pico W
let joystickData = {
    x: 0,
    y: 0,
    direction: "Centro",
    temperature: 0.0,
    button: "Liberado"
};

// Endpoint para receber dados da Raspberry Pi Pico W
app.use(express.json()); // Para que o express entenda o corpo da requisição como JSON

app.post('/update-data', (req, res) => {
    const { x, y, direction, temperature, button } = req.body;

    joystickData = { x, y, direction, temperature, button };
    console.log("Dados recebidos da Pico W:", joystickData);

    res.status(200).send("Dados recebidos com sucesso");
});

// Endpoint para fornecer os dados para a interface web
app.get('/data', (req, res) => {
    res.json(joystickData);
});

// Servir a página HTML
app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
});

app.listen(port, () => {
    console.log(`Servidor rodando em http://localhost:${port}`);
});
