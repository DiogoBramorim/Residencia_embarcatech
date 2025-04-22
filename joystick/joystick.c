// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/adc.h"
// #include "pico/cyw43_arch.h"
// #include "string.h"
// #include "stdlib.h"
// #include "lwip/pbuf.h"
// #include "lwip/tcp.h"
// #include "lwip/netif.h"

// const int valorx = 26;
// const int valory = 27;
// const int ADC_valorx = 1;
// const int ADC_valory = 0;
// const int button_joy = 22;
// const int TEMP_SENSOR_ADC_PIN = 4;

// #define led_pin CYW43_WL_GPIO_LED_PIN
// // #define WIFI "IFPI_BLOCOJ2"
// // #define WIFI_PASS "ifpi2022@blocoj"
// #define WIFI "ADRIANA ALVES"
// #define WIFI_PASS "luanalves@1"

// void setup_joy() {
//     adc_init();
//     adc_gpio_init(valorx);
//     adc_gpio_init(valory);
//     gpio_init(button_joy);
//     gpio_set_dir(button_joy, GPIO_IN);
//     gpio_pull_up(button_joy);
// }

// void setup(){
//     stdio_init_all();
//     setup_joy();
// }

// void joystick_read_axis(uint16_t *eixo_x, uint16_t *eixo_y){
//     adc_select_input(ADC_valorx); 
//     sleep_us(2);                    
//     *eixo_x = adc_read();         
//     adc_select_input(ADC_valory);
//     sleep_us(2);                
//     *eixo_y = adc_read();      
// }

// const char* get_direcao_joy(uint16_t x, uint16_t y){
//     if(y>4000) return "Norte";
//     else if(y<50) return "Sul";
//     else if (x>4000) return "Leste";
//     else if(x<50) return "Oeste";
//     else if(x > 240 && y>3900) return "Noroeste";
//     else if (x>3700 && y>3800) return "Nordeste";
//     else return "Centro";
// }
// float read_temperature() {
//     adc_select_input(TEMP_SENSOR_ADC_PIN);
//     uint16_t raw_value = adc_read();
//     const float conversion_factor = 3.3f / (1 << 12);
//     return 27.0f - ((raw_value * conversion_factor) - 0.706f) / 0.001721f;
// }
// static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
//     if (!p) {
//         tcp_close(tpcb);
//         tcp_recv(tpcb, NULL);
//         return ERR_OK;
//     }

//     char *request = (char *)malloc(p->len + 1);
//     if (request == NULL) {
//         pbuf_free(p);
//         tcp_close(tpcb);
//         tcp_recv(tpcb, NULL);
//         return ERR_MEM;
//     }
//     memcpy(request, p->payload, p->len);
//     request[p->len] = '\0';

//     printf("Request: %s\n", request);

//     float temperature = read_temperature();
//     uint16_t x, y;
//     joystick_read_axis(&x, &y);
//     const char *direcao = get_direcao_joy(x, y);

//     char html[1024];
//     snprintf(html, sizeof(html),
//              "HTTP/1.1 200 OK\r\n"
//              "Content-Type: text/html\r\n"
//              "\r\n"
//              "<!DOCTYPE html>\n"
//              "<html>\n"
//              "<head>\n"
//              "<title>Joystick e Sensor</title>\n"
//              "<meta http-equiv=\"refresh\" content=\"1\">\n"
//              "<style>\n"
//              "body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n"
//              ".data { font-size: 48px; margin-top: 30px; color: #333; }\n"
//              "</style>\n"
//              "</head>\n"
//              "<body>\n"
//              "<h1>Dashboard Joystick</h1>\n"
//              "<p class=\"data\">Temperatura Interna: %.2f &deg;C</p>\n"
//              "<p class=\"data\">Joystick Direcao: %s</p>\n"
//              "<p class=\"data\">X: %d | Y: %d</p>\n"
//              "</body>\n"
//              "</html>\n",
//              temperature, direcao, x, y);

//     err_t write_err = tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
//     if (write_err != ERR_OK) {
//         printf("Erro ao escrever dados TCP: %d\n", write_err);
//     }

//     err_t output_err = tcp_output(tpcb);
//     if (output_err != ERR_OK) {
//         printf("Erro ao enviar dados TCP: %d\n", output_err);
//     }

//     free(request);
//     pbuf_free(p);

//     return ERR_OK;
// }


// static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
// {
//     tcp_recv(newpcb, tcp_server_recv);
//     return ERR_OK;
// }

// int main()
// {
//     setup();
//     setup_joy();

//     while (cyw43_arch_init()) {
//         printf("Falha ao inicializar Wi-Fi\n");
//         sleep_ms(100);
//         return -1;
//     }

//     cyw43_arch_enable_sta_mode();
//     printf("Conectando ao Wi-Fi...\n");
//     while (cyw43_arch_wifi_connect_timeout_ms(WIFI, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 20000)) {
//         printf("Falha ao conectar ao Wi-Fi\n");
//         sleep_ms(100);
//         return -1;
//     }

//     printf("Conectado ao Wi-Fi\n");

//     if (netif_default) {
//         printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
//     }

//     struct tcp_pcb *server = tcp_new();
//     if (!server) {
//         printf("Falha ao criar servidor TCP\n");
//         return -1;
//     }

//     if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
//         printf("Falha ao associar servidor TCP à porta 80\n");
//         return -1;
//     }

//     server = tcp_listen(server);
//     tcp_accept(server, tcp_server_accept);

//     printf("Servidor ouvindo na porta 80\n");

//     adc_init();
//     adc_set_temp_sensor_enabled(true);

//     while (true) {
//         cyw43_arch_poll();
//     }

//     cyw43_arch_deinit();
//     return 0;
// }


// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"
// #include "hardware/adc.h"
// #include "lwip/tcp.h"

// #define PORTA_TCP 80
// #define TEMPO_ENVIO_MS 1000

// static struct tcp_pcb *cliente_pcb = NULL;

// // Função para formatar os dados do sensor
// void coletar_dados(char *buffer, size_t tamanho) {
//     // Leitura dos eixos do joystick (X e Y)
//     adc_select_input(0);  // Selecionando ADC0 para o eixo X
//     int x = adc_read(); sleep_ms(5);
    
//     adc_select_input(1);  // Selecionando ADC1 para o eixo Y
//     int y = adc_read(); sleep_ms(5);
    
//     // Leitura do botão
//     int botao = gpio_get(15);
    
//     // Leitura da temperatura
//     adc_select_input(4);  // Selecionando ADC4 para o sensor de temperatura
//     float temperatura = 27 - ((adc_read() * 3.3f / 4096.0f - 0.706f) / 0.001721f);
    
//     snprintf(buffer, tamanho,
//              "Joystick X: %d\nJoystick Y: %d\nBotão: %d\nTemperatura: %.2f ºC\n",
//              x, y, botao, temperatura);
// }

// // Função chamada quando o cliente se desconecta
// static void ao_encerrar_conexao(void *arg, err_t err) {
//     printf("Cliente desconectado.\n");
//     cliente_pcb = NULL;
// }

// // Função chamada quando o cliente se conecta
// static err_t ao_conectar(void *arg, struct tcp_pcb *newpcb, err_t err) {
//     printf("Cliente conectado!\n");
//     cliente_pcb = newpcb;
//     tcp_arg(newpcb, NULL);
//     tcp_err(newpcb, ao_encerrar_conexao);
//     return ERR_OK;
// }

// // Loop principal para enviar os dados a cada 1 segundo
// void loop_envio_dados() {
//     char buffer[256];

//     while (true) {
//         if (cliente_pcb) {
//             coletar_dados(buffer, sizeof(buffer));
//             err_t erro = tcp_write(cliente_pcb, buffer, strlen(buffer), TCP_WRITE_FLAG_COPY);
//             if (erro == ERR_OK) {
//                 tcp_output(cliente_pcb);
//                 printf("Dados enviados ao cliente.\n");
//             } else {
//                 printf("Erro ao enviar: %d\n", erro);
//             }
//         }
//         sleep_ms(TEMPO_ENVIO_MS);
//     }
// }

// int main() {
//     stdio_init_all();
//     if (cyw43_arch_init()) {
//         printf("Erro ao iniciar Wi-Fi\n");
//         return 1;
//     }

//     // Conectando ao Wi-Fi
//     const char *ssid = "ADRIANA ALVES";
//     const char *senha = "luanalves@1";

//     printf("Conectando ao Wi-Fi...\n");
//     cyw43_arch_enable_sta_mode();
//     if (cyw43_arch_wifi_connect_timeout_ms(ssid, senha, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
//         printf("Falha ao conectar no Wi-Fi.\n");
//         return 1;
//     }
//     printf("Conectado! IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));

//     // Inicializando os ADCs e GPIO
//     adc_init();
//     adc_gpio_init(26);  // Pino do eixo X
//     adc_gpio_init(27);  // Pino do eixo Y
//     adc_gpio_init(28);  // Pino do eixo Z (se necessário)
//     adc_select_input(0);  // Exemplo: joystick X no ADC0
//     gpio_init(15);
//     gpio_set_dir(15, GPIO_IN);  // Configura pino para leitura de botão

//     // Criando o servidor TCP
//     struct tcp_pcb *servidor_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
//     tcp_bind(servidor_pcb, IP_ANY_TYPE, PORTA_TCP);
//     servidor_pcb = tcp_listen_with_backlog(servidor_pcb, 1);
//     tcp_accept(servidor_pcb, ao_conectar);
//     printf("Servidor TCP aguardando na porta %d...\n", PORTA_TCP);

//     loop_envio_dados();  // Inicia envio contínuo
//     return 0;
// }#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"

#define PORTA_TCP 80
#define TEMPO_ENVIO_MS 1000
#define temperatura_pin 4

static struct tcp_pcb *cliente_pcb = NULL;

float read_temperature() {
    adc_select_input(temperatura_pin);  // Seleciona o pino de entrada do sensor de temperatura
    uint16_t raw_value = adc_read();    // Lê o valor bruto do ADC
    const float conversion_factor = 3.3f / (1 << 12);  // Fator de conversão para 12 bits (4096 valores)
    float voltage = raw_value * conversion_factor;  // Converte a leitura para a tensão
    return  27.0f - ((voltage - 0.706f) / 0.001721f);
}
// Função para formatar os dados do sensor
void coletar_dados(char *buffer, size_t tamanho) {
    // Leitura dos eixos do joystick (X e Y)
    adc_select_input(1);  // Selecionando ADC0 para o eixo X 
    int x = adc_read(); sleep_ms(5);
    
    adc_select_input(0);  // Selecionando ADC1 para o eixo Y 
    int y = adc_read(); sleep_ms(5);
    
    // Leitura do botão
    int botao = gpio_get(15);
    
    // Leitura da temperatura
    float temperatura = read_temperature();
    
    snprintf(buffer, tamanho,
             "Joystick X: %d\nJoystick Y: %d\nBotão: %d\nTemperatura: %.2f ºC\n",
             x, y, botao, temperatura);
}

// Função chamada quando o cliente se desconecta
static void ao_encerrar_conexao(void *arg, err_t err) {
    printf("Cliente desconectado.\n");
    cliente_pcb = NULL;
}

// Função chamada quando o cliente se conecta
static err_t ao_conectar(void *arg, struct tcp_pcb *newpcb, err_t err) {
    printf("Cliente conectado!\n");
    cliente_pcb = newpcb;
    tcp_arg(newpcb, NULL);
    tcp_err(newpcb, ao_encerrar_conexao);
    return ERR_OK;
}

// Função para enviar dados ao cliente com verificação do buffer
void enviar_dados_com_controle() {
    char buffer[256];

    // Coletar os dados
    coletar_dados(buffer, sizeof(buffer));
    
    if (cliente_pcb) {
        // Verificar o espaço no buffer de envio
        uint16_t buffer_tamanho = tcp_sndbuf(cliente_pcb); // Tamanho do buffer de envio disponível

        if (strlen(buffer) <= buffer_tamanho) {  // Só envia se o buffer tiver espaço suficiente
            err_t erro = tcp_write(cliente_pcb, buffer, strlen(buffer), TCP_WRITE_FLAG_COPY);
            if (erro == ERR_OK) {
                tcp_output(cliente_pcb);  // Forçar envio dos dados
                printf("Dados enviados ao cliente.\n");
            } else {
                printf("Erro ao enviar dados: %d\n", erro);
            }
        } else {
            printf("Sem espaço suficiente no buffer de envio. Tentando novamente...\n");
        }
    }
}

// Loop principal para enviar os dados a cada 1 segundo
void loop_envio_dados() {
    while (true) {
        if (cliente_pcb) {
            enviar_dados_com_controle();  // Envia os dados com controle de buffer
        }
        sleep_ms(TEMPO_ENVIO_MS);
    }
}

int main() {
    stdio_init_all();
    if (cyw43_arch_init()) {
        printf("Erro ao iniciar Wi-Fi\n");
        return 1;
    }

    // Conectando ao Wi-Fi
    const char *ssid = "ADRIANA ALVES";
    const char *senha = "luanalves@1";

    printf("Conectando ao Wi-Fi...\n");
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, senha, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha ao conectar no Wi-Fi.\n");
        return 1;
    }
    printf("Conectado! IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));

    // Inicializando os ADCs e GPIO
    adc_init();
    adc_gpio_init(26);  // Pino do eixo X
    adc_gpio_init(27);  // Pino do eixo Y
    adc_gpio_init(28);  // Pino do eixo Z (se necessário)
    adc_select_input(0);  // Exemplo: joystick X no ADC0
    gpio_init(15);
    gpio_set_dir(15, GPIO_IN);  // Configura pino para leitura de botão

    // Criando o servidor TCP
    struct tcp_pcb *servidor_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    tcp_bind(servidor_pcb, IP_ANY_TYPE, PORTA_TCP);
    servidor_pcb = tcp_listen_with_backlog(servidor_pcb, 1);
    tcp_accept(servidor_pcb, ao_conectar);
    printf("Servidor TCP aguardando na porta %d...\n", PORTA_TCP);

    loop_envio_dados();  // Inicia envio contínuo
    return 0;
}
