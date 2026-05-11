#ifndef ULTRASSOM_H
#define ULTRASSOM_H

#include <zephyr/kernel.h>

// Inicializa o sensor (Trigger no TPM2_CH0 e Echo no TPM1_CH0)
void ultrassom_init(void);

// Retorna a última distância lida em centímetros
float ultrassom_get_distancia(void);

#endif