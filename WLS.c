#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "wls.pio.h"

// Definições dos pinos
#define OUT_PIN 7
#define LED_R 13
#define BTN_A 5
#define BTN_B 6

static volatile uint32_t numero = 0; // Número exibido na matriz
static volatile char flag = 0;

// Frames
double contagem [10][25] = {
        {0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25},

         {0.25, 0.25, 0.25, 0.25, 0.25,
          0.00, 0.00, 0.25, 0.00, 0.00,
          0.00, 0.00, 0.25, 0.00, 0.25,
          0.00, 0.25, 0.25, 0.00, 0.00,
          0.00, 0.00, 0.25, 0.00, 0.00},
        
        {0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.00,
         0.25, 0.25, 0.25, 0.25, 0.25,
         0.00, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25},

        {0.25, 0.25, 0.25, 0.25, 0.25,
         0.00, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25,
         0.00, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25},

        {0.25, 0.00, 0.00, 0.00, 0.00,
         0.00, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25},

        {0.25, 0.25, 0.25, 0.25, 0.25,
         0.00, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.00,
         0.25, 0.25, 0.25, 0.25, 0.25},

        {0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.00,
         0.25, 0.25, 0.25, 0.25, 0.25},

        {0.25, 0.00, 0.00, 0.00, 0.00,
         0.00, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.00,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25},

        {0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25},

        {0.25, 0.25, 0.25, 0.25, 0.25,
         0.00, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25,
         0.25, 0.00, 0.00, 0.00, 0.25,
         0.25, 0.25, 0.25, 0.25, 0.25}
    };

void gpio_irq_handler(uint gpio, uint32_t events)
{
    // Tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());
  
    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - numero > 200000)
    {
        numero = current_time; // Atualiza o tempo do último evento
        if(gpio == BTN_A){
            if (flag < 9)
            flag += 1;
        }
        else {
            if (flag > 0)
                flag -= 1;
            
        }
    }
}

// Função para piscar o LED vermelho
bool piscar_led_r(struct repeating_timer *t) {
    gpio_put(LED_R, !gpio_get(LED_R));
    return true;
}

// rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double r, double g, double b) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função para exibir o número na matriz de LEDs
void exibir_numero(PIO pio, uint sm) {

    for (int i = 0; i < 25; i++) {
            uint32_t color = matrix_rgb(contagem[flag][i], contagem[flag][i], contagem[flag][i]);
            pio_sm_put_blocking(pio, sm, color);
        }

}

void setup() {
    // Inicializa os LEDs como saída
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    // Configura botões como entrada com pull-up
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    // Configura interrupções nos botões
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
}

int main()
{
    // Configurações Iniciais
    set_sys_clock_khz(128000, false);
    stdio_init_all();
    
    // Chama a função de callback a cada 1 segundo
    struct repeating_timer timer;
    add_repeating_timer_ms(200, piscar_led_r, NULL, &timer);
    
    // Configuração inicial dos GPIOs e interrupções
    setup();

    // Configurações da PIO
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &wls_program);
    uint sm = pio_claim_unused_sm(pio, true);
    wls_program_init(pio, sm, offset, OUT_PIN);

    while (true) {
        exibir_numero(pio, sm);
        sleep_ms(100);
    }
}