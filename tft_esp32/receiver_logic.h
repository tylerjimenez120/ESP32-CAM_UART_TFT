#ifndef RECEIVER_LOGIC_H
#define RECEIVER_LOGIC_H

#include <stdint.h>
#include "esp_err.h"

#define RX_PIN 16  // Pin VOR (Conectar al VOT de la ESP32-CAM)
#define TX_PIN 17  
#define BUF_SIZE 40000

void init_uart_receiver();
esp_err_t wait_for_frame(uint8_t *buffer, uint32_t *out_len);

#endif

/*
Configura pines UART (RX_PIN, TX_PIN)

Define un buffer grande (BUF_SIZE) para recibir un frame completo

init_uart_receiver() → inicializa el UART

wait_for_frame() → bloquea/espera hasta recibir un frame completo y devuelve:

datos en buffer

tamaño real en out_len
*/