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
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "string.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"

const int valorx = 26;
const int valory = 27;
const int ADC_valorx = 0;
const int ADC_valory = 1;
const int button_joy = 22;
const int TEMP_SENSOR_ADC_PIN = 4;

#define WIFI_SSID "IFPI_BLOCOJ2"
#define WIFI_PASS "ifpi2022@blocoj"
#define TCP_PORT 80

typedef struct {
    struct tcp_pcb *pcb;
    bool connected;
} tcp_client_t;

static tcp_client_t tcp_client;

void setup_joy() {
    adc_init();
    adc_gpio_init(valorx);
    adc_gpio_init(valory);
    gpio_init(button_joy);
    gpio_set_dir(button_joy, GPIO_IN);
    gpio_pull_up(button_joy);
}

float read_temperature() {
    adc_select_input(TEMP_SENSOR_ADC_PIN);
    uint16_t raw_value = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    return 27.0f - ((raw_value * conversion_factor) - 0.706f) / 0.001721f;
}

void joystick_read_axis(uint16_t *x, uint16_t *y) {
    adc_select_input(ADC_valorx);
    *x = adc_read();
    adc_select_input(ADC_valory);
    *y = adc_read();
}

const char* get_direcao_joy(uint16_t x, uint16_t y) {
    if(y > 3500) return x > 3500 ? "Nordeste" : (x < 500 ? "Noroeste" : "Norte");
    else if(y < 500) return x > 3500 ? "Sudeste" : (x < 500 ? "Sudoeste" : "Sul");
    else if(x > 3500) return "Leste";
    else if(x < 500) return "Oeste";
    else return "Centro";
}

static err_t tcp_server_send_data(struct tcp_pcb *tpcb) {
    if (!tpcb || !tcp_client.connected) return ERR_CONN;

    float temp = read_temperature();
    uint16_t x, y;
    joystick_read_axis(&x, &y);
    int button = !gpio_get(button_joy); // 1 = pressionado

    char html[1024];
    snprintf(html, sizeof(html),
        "<p><strong>Temperatura:</strong> %.2f °C</p>"
        "<p><strong>Joystick:</strong> Direção %s</p>"
        "<p><strong>X:</strong> %d &nbsp;&nbsp;&nbsp; <strong>Y:</strong> %d</p>"
        "<p><strong>Botão:</strong> %s</p>",
        temp, get_direcao_joy(x, y), x, y, button ? "Pressionado" : "Solto");

    char response[1200];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", html);

    err_t err = tcp_write(tpcb, response, strlen(response), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("Erro ao enviar: %d\n", err);
        return err;
    }

    return tcp_output(tpcb);
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_client.connected = false;
        tcp_close(tpcb);
        return ERR_OK;
    }

    pbuf_free(p);
    return tcp_server_send_data(tpcb);
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (err != ERR_OK || newpcb == NULL) return ERR_VAL;

    tcp_client.pcb = newpcb;
    tcp_client.connected = true;

    tcp_arg(newpcb, &tcp_client);
    tcp_recv(newpcb, tcp_server_recv);

    printf("Cliente conectado: %s\n", ip4addr_ntoa(&newpcb->remote_ip));
    return ERR_OK;
}

bool init_wifi() {
    if (cyw43_arch_init()) {
        printf("Erro Wi-Fi\n");
        return false;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi %s...\n", WIFI_SSID);

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 20000) == 0) {
        printf("Conectado! IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
        return true;
    }

    printf("Falha na conexão\n");
    return false;
}

bool init_tcp_server() {
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) return false;

    if (tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT) != ERR_OK) return false;

    pcb = tcp_listen(pcb);
    tcp_accept(pcb, tcp_server_accept);
    return true;
}

int main() {
    stdio_init_all();
    setup_joy();
    adc_set_temp_sensor_enabled(true);

    if (!init_wifi()) return 1;
    if (!init_tcp_server()) return 1;

    while (true) {
        cyw43_arch_poll();
    }

    return 0;
}