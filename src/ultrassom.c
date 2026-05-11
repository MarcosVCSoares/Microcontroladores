#include "ultrassom.h"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <pwm_z42.h> 

// Usamos o TPM0 porque PTD1 e PTD3 pertencem a ele
#define TPM_IRQ_LINE TPM0_IRQn
#define PORTA_D_NODE DT_NODELABEL(gpiod)

volatile uint16_t t_subida = 0;
volatile uint16_t t_descida = 0;
volatile float dist_cm = 400.0;
const struct device *dev_d;

// Função de Interrupção (ISR) para medir o tempo do pulso Echo
void tpm0_isr_hc(void *arg) {
    // Limpa a flag de interrupção do Canal 3 (PTD3)
    TPM0->STATUS |= TPM_STATUS_CH3F_MASK; 
    uint16_t cap = TPM0->CONTROLS[3].CnV; 

    // Se o pino PTD3 estiver em nível alto, é a borda de subida
    if (gpio_pin_get_raw(dev_d, 3)) {
        t_subida = cap;
    } 
    // Se estiver em nível baixo, é a borda de descida
    else {
        t_descida = cap;
        
        // Cálculo da diferença de tempo considerando o MOD 40000
        uint16_t diff;
        if (t_descida >= t_subida) {
            diff = t_descida - t_subida;
        } else {
            diff = (40000 - t_subida) + t_descida;
        }
        
        // Conversão para centímetros:
        // (diff * tempo_por_tick) / 58
        dist_cm = (diff * 2.666f) / 58.0f;
    }
}

void ultrassom_init(void) {
    // 1. Pega o dispositivo da Porta D
    dev_d = DEVICE_DT_GET(PORTA_D_NODE);

    // 2. Conecta e habilita a interrupção do TPM0
    IRQ_CONNECT(TPM_IRQ_LINE, 1, tpm0_isr_hc, NULL, 0);
    irq_enable(TPM_IRQ_LINE);

    // 3. Inicializa o TPM0 com o MOD 40000 
    // (Isso sincroniza com o PWM dos motores no main.c)
    pwm_tpm_Init(TPM0, TPM_PLLFLL, 40000, TPM_CLK, PS_128, EDGE_PWM);
    
    // 4. Configura PTD3 como Entrada de Captura (Echo)
    // 0x0C = Captura em ambas as bordas | 0x40 = Habilita interrupção do canal
    pwm_tpm_Ch_Init(TPM0, 3, (0x0C | 0x40), GPIOD, 3);

    // 5. Configura PTD1 como Saída PWM (Trigger)
    pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOD, 1);
    
    // 6. Define um pulso de Trigger estável (aprox. 15-20us)
    pwm_tpm_CnV(TPM0, 1, 15); 
}

float ultrassom_get_distancia(void) {
    return dist_cm;
}