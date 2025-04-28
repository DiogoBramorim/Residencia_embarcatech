#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/tcp.h"

#define PORTA_TCP 80
#define TEMPO_ENVIO_MS 1000



static struct tcp_pcb *cliente_pcb = NULL;

// Função para determinar a direção do joystick
const char* determinar_direcao(int x, int y) {
    const int DEADZONE = 1000; // zona morta para evitar ruído em torno do centro
    const int MAX_ADC = 4095 / 2; // valor central aproximado

    int deltaX = x - MAX_ADC;
    int deltaY = y - MAX_ADC;

    if (abs(deltaX) < DEADZONE && abs(deltaY) < DEADZONE) {
        return "Centro";
    } else if (abs(deltaX) < DEADZONE) {
        return (deltaY > 0) ? "Sul" : "Norte";
    } else if (abs(deltaY) < DEADZONE) {
        return (deltaX > 0) ? "Leste" : "Oeste";
    } else if (deltaX > 0 && deltaY > 0) {
        return "Nordeste";
    } else if (deltaX < 0 && deltaY > 0) {
        return "Noroeste";
    } else if (deltaX > 0 && deltaY < 0) {
        return "Sudeste";
    } else {
        return "Sudoeste";
    }
}

// Função para formatar os dados do sensor
void coletar_dados(char *buffer, size_t tamanho) {
    // Leitura dos eixos do joystick (X e Y)
    adc_select_input(1);  // Selecionando ADC1 para o eixo X 
    int x = adc_read(); sleep_ms(3);
    
    adc_select_input(0);  // Selecionando ADC0 para o eixo Y 
    int y = adc_read(); sleep_ms(3);

    // Determinar a direção
    const char* direcao = determinar_direcao(x, y);

    snprintf(buffer, tamanho,
             "Joystick X: %d\nJoystick Y: %d\nDirecao: %s\n",
             x, y, direcao);
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

    // Inicializando os ADCs e GPIO
    adc_init();
    adc_gpio_init(26);  // Pino do eixo X
    adc_gpio_init(27);  // Pino do eixo Y
    adc_gpio_init(28); 
    adc_select_input(0);  
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
