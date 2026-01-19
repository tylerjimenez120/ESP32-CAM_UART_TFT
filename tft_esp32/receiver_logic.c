#include "receiver_logic.h"
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "RECEIVER";

void init_uart_receiver() {
    uart_config_t uart_config = {
        .baud_rate = 2000000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    //Configura UART rápido (2 Mbps), 8N1, sin flow control → ideal para frames grandes (JPEG).
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_2, BUF_SIZE * 2, 0, 0, NULL, 0);
    //Usa UART2 Asigna pines TX/RX definidos en el .h
    //Instala driver con buffer RX grande
    // No usa colas ni eventos (modo simple)
}

esp_err_t wait_for_frame(uint8_t *buffer, uint32_t *out_len) {
    uint8_t marker[3];
    // Buscamos el marcador de inicio
    if (uart_read_bytes(UART_NUM_2, marker, 3, pdMS_TO_TICKS(10)) == 3) {
        if (marker[0] == 0xAA && marker[1] == 0xBB && marker[2] == 0xCC) {
            // Leemos los 4 bytes del tamaño
            uart_read_bytes(UART_NUM_2, (uint8_t*)out_len, 4, pdMS_TO_TICKS(50));
            
            if (*out_len < BUF_SIZE) {
                // Leemos el payload (JPEG)
                return (uart_read_bytes(UART_NUM_2, buffer, *out_len, pdMS_TO_TICKS(500)) == *out_len) ? ESP_OK : ESP_FAIL;
            }
        }
    }
    return ESP_ERR_NOT_FOUND;
}

/*
UART rápido

Protocolo simple: MARKER + SIZE + DATA

Función bloqueante

Diseñado para streaming de imágenes*/