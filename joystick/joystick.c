#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include "string.h"
#include "stdlib.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h" // Para acessar netif_default e IP

// Pin joystick
const int valorx = 26;
const int valory = 27;
const int ADC_valorx = 1;
const int ADC_valory = 0;
const int button_joy = 22;

// Pin Wi-fi
#define led_pin CYW43_WL_GPIO_LED_PIN
#define WIFI "IFPI_BLOCOJ2"
#define WIFI_PASS "ifpi2022@blocoj"

void setup_joy(){
    adc_init();
    adc_gpio_init(valorx);
    adc_gpio_init(valory);

    gpio_init(button_joy);
    gpio_set_dir(button_joy, GPIO_IN);
    gpio_pull_up(button_joy);
}

void setup(){
    stdio_init_all();
    setup_joy();
}
void joystick_read_axis(uint16_t *eixo_x, uint16_t *eixo_y){
    // Leitura do valor do eixo X do joystick
    adc_select_input(ADC_valorx); 
    sleep_us(2);                    
    *eixo_x = adc_read();         
  
    // Leitura do valor do eixo Y do joystick
    adc_select_input(ADC_valory);
    sleep_us(2);                
    *eixo_y = adc_read();      
  }
  const char* get_direcao_joy(uint16_t x, uint16_t y){
      if(y>4000) return "Norte";
      else if(y<50) return "Sul";
      else if (x>4000) return "Leste";
      else if(x<50) return "Oeste";
      else if(x > 240 && y>3900) return "Noroeste";
      else if (x>3700 && y>3800) return "Nordeste";
      else return "Centro";
  }
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("Request: %s\n", request);

    // Leitura da temperatura interna
    adc_select_input(4);
    uint16_t raw_value = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    float temperature = 27.0f - ((raw_value * conversion_factor) - 0.706f) / 0.001721f;


    uint16_t x, y;
    joystick_read_axis(&x, &y);
    const char* direcao = get_direcao_joy(x, y);
    // Cria a resposta HTML
    char html[1024];

    snprintf(html, sizeof(html),
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "<title>Joystick e Sensor</title>\n"
    "<style>\n"
    "body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }\n"
    "h1 { font-size: 64px; margin-bottom: 30px; }\n"
    ".data { font-size: 48px; margin-top: 30px; color: #333; }\n"
    "</style>\n"
    "<script>\n"
    "function updateData() {\n"
    "    fetch('/data')\n"
    "        .then(response => response.json())\n"
    "        .then(data => {\n"
    "            document.getElementById('temperature').innerText = `Temperatura Interna: ${data.temperature.toFixed(2)} °C`;\n"
    "            document.getElementById('direction').innerText = `Joystick Direcao: ${data.direction}`;\n"
    "            document.getElementById('coordinates').innerText = `X: ${data.x} | Y: ${data.y}`;\n"
    "        });\n"
    "}\n"
    "setInterval(updateData, 1000);\n"
    "</script>\n"
    "</head>\n"
    "<body>\n"
    "<p class=\"data\" id=\"temperature\">Temperatura Interna: %.2f &deg;C</p>\n"
    "<p class=\"data\" id=\"direction\">Joystick Direcao: %s</p>\n"
    "<p class=\"data\" id=\"coordinates\">X: %d | Y: %d</p>\n"
    "</body>\n"
    "</html>\n",
    temperature, direcao, x, y);

    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    free(request);
    pbuf_free(p);

    return ERR_OK;
}

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}
int main()
{
    setup();
    setup_joy();
    while (cyw43_arch_init())
    {
        printf("Falha ao inicializar Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 20000))
    {
        printf("Falha ao conectar ao Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }

    printf("Conectado ao Wi-Fi\n");

    if (netif_default)
    {
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }

    // Configura o servidor TCP
    struct tcp_pcb *server = tcp_new();
    if (!server)
    {
        printf("Falha ao criar servidor TCP\n");
        return -1;
    }

    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Falha ao associar servidor TCP à porta 80\n");
        return -1;
    }

    server = tcp_listen(server);
    tcp_accept(server, tcp_server_accept);

    printf("Servidor ouvindo na porta 80\n");

    // Inicializa o ADC
    adc_init();
    adc_set_temp_sensor_enabled(true);

    while (true)
    {
        cyw43_arch_poll();
    }

    cyw43_arch_deinit();
    return 0;
}
