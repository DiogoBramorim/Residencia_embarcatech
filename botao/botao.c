#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"

#define PORTA_TCP 80
#define TEMPO_ENVIO_MS 1000
#define botton_A 5
#define botton_B 6
// #define temperatura_pin 4

static struct tcp_pcb *cliente_pcb = NULL;

void coletar_dados(char *buffer, size_t tamanho) {

    // Leitura do botão
    int botao_a = gpio_get(botton_A);
    int botao_b = gpio_get(botton_B);
    // Leitura da temperatura interna
    adc_select_input(4);
    uint16_t raw_value = adc_read();
    const float conversion_factor = 3.3f / (1 << 12);
    float temperature = 27.0f - ((raw_value * conversion_factor) - 0.706f) / 0.001721f;
    // // Leitura da temperatura
    // float temperatura = read_temperature();
    snprintf(buffer, tamanho,
             "BOTÃO A: %d\nBOTÃO B: %d\nTEMPERATURA: %.2f\n",
             botao_a, botao_b, temperature);
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
    // const char *ssid = "ADRIANA ALVES";
    // const char *senha = "luanalves@1";
    const char *ssid = "IFPI_BLOCOJ2";
    const char *senha = "ifpi2022@blocoj";

    printf("Conectando ao Wi-Fi...\n");
    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, senha, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Falha ao conectar no Wi-Fi.\n");
        return 1;
    }
    printf("Conectado! IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));

    gpio_init(botton_A); // Iniciando o Botão
    gpio_set_dir(botton_A, GPIO_IN);
    gpio_pull_up(botton_A);
    gpio_init(botton_B); // Iniciando o Botão
    gpio_set_dir(botton_B, GPIO_IN);
    gpio_pull_up(botton_B);
    // Inicializa o ADC
    adc_init();
    adc_set_temp_sensor_enabled(true);
    // Criando o servidor TCP
    struct tcp_pcb *servidor_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    tcp_bind(servidor_pcb, IP_ANY_TYPE, PORTA_TCP);
    servidor_pcb = tcp_listen_with_backlog(servidor_pcb, 1);
    tcp_accept(servidor_pcb, ao_conectar);
    printf("Servidor TCP aguardando na porta %d...\n", PORTA_TCP);

    loop_envio_dados();  // Inicia envio contínuo
    return 0;
}
