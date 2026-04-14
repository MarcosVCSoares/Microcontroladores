#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led2)

static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

// Definição dos estados
typedef enum {
    STATE_LED1_TOGGLE,
    STATE_WAIT_LED1,
    STATE_LED2_TOGGLE,
    STATE_WAIT_LED2,
} state_t;

void main(void)
{
    int ret;
    state_t current_state = STATE_LED1_TOGGLE;
    int64_t next_event_time = 0;

    // Validação de Hardware
    if (!gpio_is_ready_dt(&led1) || !gpio_is_ready_dt(&led2)) {
        return;
    }

    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);

    printk("Iniciando Máquina de Estados...\n");

    while (1) {
        switch (current_state) {
            
            case STATE_LED1_TOGGLE:
                gpio_pin_toggle_dt(&led1);
                // Define quando o próximo evento deve ocorrer
                next_event_time = k_uptime_get() + 3000; 
                current_state = STATE_WAIT_LED1;
                break;

            case STATE_WAIT_LED1:
                if (k_uptime_get() >= next_event_time) {
                    // Após 3s, inverte LED1 e LED2 simultaneamente como no original
                    gpio_pin_toggle_dt(&led1);
                    gpio_pin_toggle_dt(&led2);
                    next_event_time = k_uptime_get() + 500;
                    current_state = STATE_WAIT_LED2;
                }
                break;

            case STATE_WAIT_LED2:
                if (k_uptime_get() >= next_event_time) {
                    gpio_pin_toggle_dt(&led2);
                    next_event_time = k_uptime_get() + 3000;
                    current_state = STATE_LED1_TOGGLE;
                }
                break;

            default:
                current_state = STATE_LED1_TOGGLE;
                break;
        }

        // Pequeno yield para não sobrecarregar a CPU e permitir outras threads
        k_yield(); 
    }
}