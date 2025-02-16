// ========================================================================================================
// BIBLIOTECAS E VARIÁVEIS GLOBAIS
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/init.h"
#include "hardware/i2c.h"
#include "hardware/watchdog.h"
#include "inc/ssd1306.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

// LEDS
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

// BOTÕES
#define BTN_A_PIN 5
#define BTN_B_PIN 6

// DISPLAY OLED
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

//BUZZER
#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 300

#define LED_COUNT 25
#define LED_PIN 7

// CONFIGURAÇÃO DE CONEXÃO
#define WIFI_SSID "Varela2_2G"
#define WIFI_PASS "1113151722"
#define CLOUD_HOST "joaorobertoembarcatech.pythonanywhere.com"
#define CLOUD_PORT 80

struct tcp_pcb *tcp_client_pcb;
ip_addr_t server_ip;

char *cargo; // Armazena "Bolsista" ou "Visitante"

// MATRIZ DE LEDS
struct pixel_t {
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};

typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;
// ========================================================================================================

// ========================================================================================================
// Declarações das funções
void setup();
static err_t http_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t http_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err);
static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
void desligar_led();
void ligar_led();
void ligar_vermelho();
void ligar_verde();
void ligar_azul();
void ligar_amarelo();

void frame_error_wifi();          // Exibe "Erro:" / "Falha Wi-Fi!"
void frame_connecting();          // Exibe "Verificando" / "Conexao..."
void frame_welcome();             // Exibe "Bem vindo ao" / "CNAT MAKER"
void frame_selection();           // Exibe opção de escolha (Bolsista ou Visitante)
void frame_bolsista_registered(); // Exibe "Bolsista" / "registrado!"
void frame_visitante_registered();// Exibe "Visitante" / "registrado!"
void pwm_init_buzzer(uint pin); 
void beep(uint pin, uint duration_ms);
void setas();

void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear();
void npWrite();
// ========================================================================================================

// ========================================================================================================
// PROGRAMA PRINCIPAL
int main() {
       
    setup();

    npInit(LED_PIN);
    npClear();
    pwm_init_buzzer(BUZZER_PIN);

    // ====================================================================================================
    // INICIALIZAÇÃO DO WI‑FI
    if (cyw43_arch_init()) {
        printf("Falha ao iniciar Wi‑Fi\n");
        frame_error_wifi();
        ligar_vermelho();
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Verificando conexao\n");
    frame_connecting();
    ligar_amarelo();

    if (cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_MIXED_PSK)) {
        printf("Falha ao conectar ao Wi‑Fi\n");
        frame_error_wifi();
        ligar_vermelho();
        return 1;
    }

    ligar_verde();
    sleep_ms(500); // Meio segundo
    desligar_led();
    // ====================================================================================================

    // ====================================================================================================
    // LÓGICA DA APLICAÇÃO
    frame_welcome();
    sleep_ms(3000);

    uint32_t previousMillis = 0;
    uint32_t interval = 2000; // Intervalo de 2 segundos
    uint8_t ledState = 0;

    while (true) {   
        frame_selection();
        
        // Obtém o tempo atual em milissegundos
        uint32_t currentMillis = time_us_32() / 1000;

        // Verifica se passaram 2 segundos desde a última transição
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;

            // Alterna o estado dos LEDs
            switch (ledState) {
                case 0:
                    // Primeiro padrão de LEDs
                    npSetLED(10, 100, 100, 100); 
                    npSetLED(11, 100, 100, 100); 
                    npSetLED(12, 100, 100, 100); 
                    npSetLED(13, 100, 100, 100); 
                    npSetLED(14, 100, 100, 100); 
                    npSetLED(16, 100, 100, 100); 
                    npSetLED(6, 100, 100, 100); 
                    npWrite();
                    ledState = 1;
                    break;

                case 1:
                    // Desliga os LEDs do primeiro padrão
                    npSetLED(10, 0, 0, 0); 
                    npSetLED(11, 0, 0, 0); 
                    npSetLED(12, 0, 0, 0); 
                    npSetLED(13, 0, 0, 0); 
                    npSetLED(14, 0, 0, 0); 
                    npSetLED(16, 0, 0, 0); 
                    npSetLED(6, 0, 0, 0); 
                    npWrite();
                    ledState = 2;
                    break;

                case 2:
                    // Segundo padrão de LEDs
                    npSetLED(10, 100, 100, 100); 
                    npSetLED(11, 100, 100, 100); 
                    npSetLED(12, 100, 100, 100); 
                    npSetLED(13, 100, 100, 100); 
                    npSetLED(14, 100, 100, 100); 
                    npSetLED(18, 100, 100, 100); 
                    npSetLED(8, 100, 100, 100); 
                    npWrite();
                    ledState = 3;
                    break;

                case 3:
                    // Desliga os LEDs do segundo padrão
                    npSetLED(10, 0, 0, 0); 
                    npSetLED(11, 0, 0, 0); 
                    npSetLED(12, 0, 0, 0); 
                    npSetLED(13, 0, 0, 0); 
                    npSetLED(14, 0, 0, 0); 
                    npSetLED(18, 0, 0, 0); 
                    npSetLED(8, 0, 0, 0);
                    npWrite();
                    ledState = 0;
                    break;
            }
        }

        // Verifica os botões continuamente
        if (gpio_get(BTN_A_PIN) == 0) {
            cargo = "Bolsista";
            break;
        } 
        else if (gpio_get(BTN_B_PIN) == 0) {
            cargo = "Visitante";
            break;
        }
    }
    npSetLED(10, 0, 0, 0); 
    npSetLED(11, 0, 0, 0); 
    npSetLED(12, 0, 0, 0); 
    npSetLED(13, 0, 0, 0); 
    npSetLED(14, 0, 0, 0); 
    npSetLED(18, 0, 0, 0); 
    npSetLED(8, 0, 0, 0);
    npSetLED(16, 0, 0, 0); 
    npSetLED(6, 0, 0, 0);
    npWrite();

    dns_gethostbyname(CLOUD_HOST, &server_ip, dns_callback, NULL);

    if (cargo != NULL && strcmp(cargo, "Bolsista") == 0) {
        frame_bolsista_registered();
    } else {
        frame_visitante_registered();
    }
    sleep_ms(2000);

    desligar_led();
    // REBOOT
    watchdog_reboot(0, 0, 0);
    // ====================================================================================================
    return 0;
}
// ========================================================================================================

// ========================================================================================================
// FUNÇÕES GERAIS
void setup() {
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);
    
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();
    stdio_init_all(); 
}
// ========================================================================================================



// ========================================================================================================
// FUNÇÕES DOS FRAMES

// Frame para erro de Wi‑Fi
void frame_error_wifi(){
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 5, 0, "Erro:");
    ssd1306_draw_string(ssd, 5, 10, "Falha Wi-Fi!");
    render_on_display(ssd, &frame_area);
}

// Frame para conexão em andamento
void frame_connecting(){
    
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 5, 0, "Verificando");
    ssd1306_draw_string(ssd, 5, 10, "Conexao...");
    render_on_display(ssd, &frame_area);
}

// Frame de boas-vindas
void frame_welcome(){
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 5, 0, "Bem vindo ao");
    ssd1306_draw_string(ssd, 5, 10, "CNAT MAKER");
    render_on_display(ssd, &frame_area);
}

// Frame de seleção: escolha entre Bolsista ou Visitante
void frame_selection(){
    
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    // Texto para a seleção (aperte A ou B)
    char *text[] = {
        "BOLSISTA APERTE",
        "",
        "      A",
        "",
        "VISITANTE APERTE",
        "",
        "      B"
    };
    int y = 0;
    for (uint i = 0; i < sizeof(text)/sizeof(text[0]); i++){
        ssd1306_draw_string(ssd, 5, y, text[i]);
        y += 8;
    }
    render_on_display(ssd, &frame_area);
}

// Frame para registro de Bolsista
void frame_bolsista_registered(){
                 // Atualiza os LEDs com as novas cores.
    sleep_ms(500);
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 5, 0, "Bolsista");
    ssd1306_draw_string(ssd, 5, 10, "registrado!");
    render_on_display(ssd, &frame_area);
}

// Frame para registro de Visitante
void frame_visitante_registered(){
    struct render_area frame_area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 5, 0, "Visitante");
    ssd1306_draw_string(ssd, 5, 10, "registrado!");
    render_on_display(ssd, &frame_area);
}
// ========================================================================================================



// ========================================================================================================
// FUNÇÕES PARA CONEXÃO HTTP COM A NUVEM
static err_t http_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }
    printf("Resposta do Servidor: %.*s\n", p->len, (char *)p->payload);
    pbuf_free(p);
    return ERR_OK;
}

static err_t http_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("Erro na conexao TCP\n");
        return err;
    }

    printf("Conectado ao Servidor!\n");
    
    npSetLED(10, 0, 0, 0); 
    npSetLED(6, 100, 100, 100); 
    npSetLED(7,100, 100, 100); 
    npSetLED(8, 100, 100, 100); 
    npSetLED(18, 100, 100, 100); 
    npSetLED(16, 100, 100, 100);
    npWrite();
    sleep_ms(100);
    
    npSetLED(10, 0, 0, 0); 
    npSetLED(6, 100, 100, 100); 
    npSetLED(7,100, 100, 100); 
    npSetLED(8, 100, 100, 100); 
    npSetLED(18, 100, 100, 100); 
    npSetLED(16, 0, 0, 0);
    beep(BUZZER_PIN, 300);
    npWrite();
    sleep_ms(1000);
    npSetLED(10, 0, 0, 0); 
    npSetLED(6, 100, 100, 100); 
    npSetLED(7,100, 100, 100); 
    npSetLED(8, 100, 100, 100); 
    npSetLED(18, 100, 100, 100); 
    npSetLED(16, 100, 100, 100);
    npWrite();
    sleep_ms(1000);
    npSetLED(10, 0, 0, 0); 
    npSetLED(6, 0, 0, 0); 
    npSetLED(7,0, 0, 0); 
    npSetLED(8, 0, 0, 0); 
    npSetLED(18, 0, 0, 0); 
    npSetLED(16, 0, 0, 0);
    npWrite();
   

    char request[512];  // Buffer para armazenar a requisicao
    char body[256];     // Buffer para o corpo da requisicao
    
    // Criar o corpo JSON da requisicao
    snprintf(body, sizeof(body),
        "{\"cargo\": \"%s\"}",
        cargo
    );
    
    // Criar o cabecalho e adicionar o corpo
    snprintf(request, sizeof(request),
        "POST /api/registro/ HTTP/1.1\r\n"
        "Host: joaorobertoembarcatech.pythonanywhere.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(body), body
    );

    // Enviar a requisicao
    tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    tcp_recv(tpcb, http_recv_callback);

    return ERR_OK;
}

static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr) {
        printf("Endereco IP da Cloud: %s\n", ipaddr_ntoa(ipaddr));

        tcp_client_pcb = tcp_new();
        tcp_connect(tcp_client_pcb, ipaddr, CLOUD_PORT, http_connected_callback);
    } else {
        printf("Falha na resolucao de DNS\n");
    }
}
// ========================================================================================================



// ========================================================================================================
// FUNÇÕES PARA LEDS
void ligar_vermelho() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
}

void ligar_verde() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
}

void ligar_azul() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 1);
}

void ligar_amarelo() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
}

void ligar_led() {
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 1);
}

void desligar_led() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
}
// ========================================================================================================



// ========================================================================================================
// FUNÇÕES PARA BUZZER
void pwm_init_buzzer(uint pin) {
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o PWM com frequência desejada
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096)); // Divisor de clock
    pwm_init(slice_num, &config, true);

    // Iniciar o PWM no nível baixo
    pwm_set_gpio_level(pin, 0);
}
void beep(uint pin, uint duration_ms){
    // Obter o slice do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);

    // Configurar o duty cycle para 50% (ativo)
    pwm_set_gpio_level(pin, 2048);

    // Temporização
    sleep_ms(duration_ms);

    // Desativar o sinal PWM (duty cycle 0)
    pwm_set_gpio_level(pin, 0);

    // Pausa entre os beeps
    sleep_ms(100); // Pausa de 100ms
}
// ========================================================================================================


// ========================================================================================================
// FUNÇÕES PARA MATRIZ DE LEDS
/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {

    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;

    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }

    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}

void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}
// ========================================================================================================