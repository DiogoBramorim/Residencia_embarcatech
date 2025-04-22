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
