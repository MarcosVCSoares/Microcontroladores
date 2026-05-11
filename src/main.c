//ATIVIDADE 3 Marcos Vinicius Camargo Soares nº 16887980
/*#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <pwm_z42.h>

#define TPM_MODULE 1000 
uint16_t duty_50  = TPM_MODULE/2; //Regulagem do PWM para o LED
uint16_t duty_90  = TPM_MODULE/1.2; //Regulagem do PWM para o LED
uint16_t velocidade = 800; // velocidade (0 (parado duty 0%) a 1000(vel. maxima duty 100%))

int main(void)
{
    // Inicializa os modulos TPM
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM1, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    // Inicializa os canais PWM
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18); // LED VERMELHO
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19); // LED VERDE
    
    // Configuração das rodas todos mode H PTA4, PTA5, PTA12 e PTD4
    pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOA, 4);  // RODA ESQUERDA - TRÁS
    pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOA, 5);  // RODA ESQUERDA - FRENTE
    pwm_tpm_Ch_Init(TPM1, 0, TPM_PWM_H, GPIOA, 12); // RODA DIREITA - TRÁS
    pwm_tpm_Ch_Init(TPM0, 4, TPM_PWM_H, GPIOD, 4);  // RODA DIREITA - FRENTE

    // LED LARANJA (Mistura de Vermelho e Verde no RGB)
    pwm_tpm_CnV(TPM2, 0, duty_50); 
    pwm_tpm_CnV(TPM2, 1, duty_90); 

    for (;;)
    {
        // PARA FRENTE
        pwm_tpm_CnV(TPM0, 1, 0);          // Esquerda Trás OFF
        pwm_tpm_CnV(TPM0, 2, velocidade); // Esquerda Frente ON
        pwm_tpm_CnV(TPM1, 0, 0);          // Direita Trás OFF
        pwm_tpm_CnV(TPM0, 4, velocidade); // Direita Frente ON
        k_msleep(3000);                   // Anda por 3 segundos

        // PARA 1 SEGUNDO
        pwm_tpm_CnV(TPM0, 2, 0); 
        pwm_tpm_CnV(TPM0, 4, 0); 
        k_msleep(1000);                   // Espera 1 segundo parado

        // PARA TRÁS 
        pwm_tpm_CnV(TPM0, 1, velocidade); // Esquerda Trás ON
        pwm_tpm_CnV(TPM0, 2, 0);          // Esquerda Frente OFF
        pwm_tpm_CnV(TPM1, 0, velocidade); // Direita Trás ON
        pwm_tpm_CnV(TPM0, 4, 0);          // Direita Frente OFF
        k_msleep(3000);                   // Volta por 3 segundos

        //PARADA
        pwm_tpm_CnV(TPM0, 1, 0); 
        pwm_tpm_CnV(TPM1, 0, 0); 
        k_msleep(1000);
    }

    return 0;
}*/

/*#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <pwm_z42.h>

// Mapeamento da Porta E (onde estão os sensores)
#define PORTA_E_NODE DT_NODELABEL(gpioe)
#define PERSISTENCIA_CURVA 2

#define TPM_MODULE 1000 
uint16_t v_cruzeiro_esq = 1000; // velocidade roda dir
uint16_t v_cruzeiro_dir = 960; // Velocidade roda esq
uint16_t v_curva    = 1000; // Velocidade para corrigir a rota

uint16_t memoria_esq = 0;
uint16_t memoria_dir = 0;

int main(void) // Correção para compilar sem erros no Zephyr
{
    // Obtém o dispositivo da Porta E
    const struct device *gpio_dev = DEVICE_DT_GET(PORTA_E_NODE);

    // --- INICIALIZAÇÃO DOS MOTORES (PWM) ---
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM1, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    // Configuração dos pinos das rodas
    pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOA, 4);  // Esq Trás
    pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOA, 5);  // Esq Frente
    pwm_tpm_Ch_Init(TPM1, 0, TPM_PWM_H, GPIOA, 12); // Dir Trás
    pwm_tpm_Ch_Init(TPM0, 4, TPM_PWM_H, GPIOD, 4);  // Dir Frente

    // --- CONFIGURAÇÃO DOS LEDS (Laranja Fixo) ---
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18); 
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19); 
    pwm_tpm_CnV(TPM2, 0, 500); 
    pwm_tpm_CnV(TPM2, 1, 830); 

    // --- CONFIGURAÇÃO DOS SENSORES (ENTRADA) ---
    gpio_pin_configure(gpio_dev, 20, GPIO_INPUT); // Sensor Esq 
    gpio_pin_configure(gpio_dev, 21, GPIO_INPUT); // Sensor Dir 

    while (1) {
        int s_esq = gpio_pin_get(gpio_dev, 20);
        int s_dir = gpio_pin_get(gpio_dev, 21);

        // --- REGRA 1: ATUALIZAÇÃO DAS MEMÓRIAS ---
        if (s_esq == 1 && s_dir == 1) {
            // Cruzamento: Cancela tudo para ir reto
            memoria_esq = 0;
            memoria_dir = 0;
        } 
        else if (s_esq == 1) {
            // Viu na esquerda: Renova curva esquerda e zera a direita
            memoria_esq = PERSISTENCIA_CURVA;
            memoria_dir = 0; 
        } 
        else if (s_dir == 1) {
            // Viu na direita: Renova curva direita e zera a esquerda
            memoria_dir = PERSISTENCIA_CURVA;
            memoria_esq = 0;
        }

        // --- REGRA 2: MOVIMENTAÇÃO ---
        if (memoria_esq > 0) {
            // CURVA PARA ESQUERDA
            pwm_tpm_CnV(TPM0, 2, 0);       // Esq Frente DESLIGADO
            pwm_tpm_CnV(TPM0, 1, 680);     // Esq Trás LIGADO (Pivô)
            pwm_tpm_CnV(TPM0, 4, v_curva); // Dir Frente LIGADO MAX
            pwm_tpm_CnV(TPM1, 0, 0);       // Dir Trás DESLIGADO 
            memoria_esq--; 
            memoria_dir = 0;
            
        }
        else if (memoria_dir > 0) {
            // CURVA PARA DIREITA
            pwm_tpm_CnV(TPM0, 2, v_curva); // Esq Frente LIGADO MAX
            pwm_tpm_CnV(TPM0, 1, 0);       // Esq Trás DESLIGADO 
            pwm_tpm_CnV(TPM0, 4, 0);       // Dir Frente DESLIGADO
            pwm_tpm_CnV(TPM1, 0, 670);     // Dir Trás LIGADO (Pivô)
            memoria_dir--;
            memoria_esq = 0;
            
        }
        else {
            // SEGUE RETO 
            pwm_tpm_CnV(TPM0, 2, v_cruzeiro_esq); // Esq Frente LIGADO
            pwm_tpm_CnV(TPM0, 4, v_cruzeiro_dir); // Dir Frente LIGADO
            pwm_tpm_CnV(TPM0, 1, 0);          // Esq Trás DESLIGADO
            pwm_tpm_CnV(TPM1, 0, 0);          // Dir Trás DESLIGADO
        }
 
    }
    
    return 0; // Fechamento obrigatório da função int main
}*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <pwm_z42.h>
#include "ultrassom.h"

#define MOD_GERAL 40000 

// Velocidades para ir reto (Base 40000)
uint16_t v_reto_esq = 32000; 
uint16_t v_reto_dir = 30000; 

int main(void) {
    // 1. Inicializa o Ultrassom (Configura TPM0 para 40000)
    ultrassom_init();

    // 2. Inicializa os outros Timers
    pwm_tpm_Init(TPM1, TPM_PLLFLL, MOD_GERAL, TPM_CLK, PS_128, EDGE_PWM);
    pwm_tpm_Init(TPM2, TPM_PLLFLL, MOD_GERAL, TPM_CLK, PS_128, EDGE_PWM);

    // 3. Configuração dos Motores
    pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOA, 5);  // Esq Frente
    pwm_tpm_Ch_Init(TPM0, 0, TPM_PWM_H, GPIOC, 1);  // Esq Trás
    pwm_tpm_Ch_Init(TPM0, 4, TPM_PWM_H, GPIOD, 4);  // Dir Frente
    pwm_tpm_Ch_Init(TPM1, 0, TPM_PWM_H, GPIOA, 12); // Dir Trás

    // LED de Status (Ligado indica que o código está rodando)
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18); 
    pwm_tpm_CnV(TPM2, 0, 5000); 

    printk("--- Modo Apenas Sensor de Distancia Ativo ---\n");

    while (1) {
        float dist = ultrassom_get_distancia();
        
        // Print para você ver no monitor serial se está medindo certo
        printk("Distancia: %d cm\n", (int)dist);

        if (dist > 0 && dist < 22) {
            // MUITO PERTO: RÉ
            pwm_tpm_CnV(TPM0, 2, 0);      pwm_tpm_CnV(TPM0, 0, 32000);
            pwm_tpm_CnV(TPM0, 4, 0);      pwm_tpm_CnV(TPM1, 0, 30000);
        }
        else if (dist >= 22 && dist <= 28.5) {
            // NO ALVO (20cm): PARADO
            pwm_tpm_CnV(TPM0, 2, 0);      pwm_tpm_CnV(TPM0, 0, 0);
            pwm_tpm_CnV(TPM0, 4, 0);      pwm_tpm_CnV(TPM1, 0, 0);
        }
        else if (dist > 28.5 && dist < 400.0) {
            // LONGE: VAI RETO
            pwm_tpm_CnV(TPM0, 2, v_reto_esq); pwm_tpm_CnV(TPM0, 0, 0);
            pwm_tpm_CnV(TPM0, 4, v_reto_dir); pwm_tpm_CnV(TPM1, 0, 0);
        }
        else {
            // Caso de erro (leitura 0 ou 400): Por segurança, PARA.
            pwm_tpm_CnV(TPM0, 2, 0);      pwm_tpm_CnV(TPM0, 0, 0);
            pwm_tpm_CnV(TPM0, 4, 0);      pwm_tpm_CnV(TPM1, 0, 0);
        }

        k_msleep(50); // Delay curto para resposta rápida
    }
    return 0;
}