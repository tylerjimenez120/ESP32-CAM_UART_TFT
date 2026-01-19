#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "camera_handler.h"
#include "esp_log.h"

#define TX_PIN 1 // Pin VOT en la base MB
#define RX_PIN 3 // Pin VOR
#define BUF_SIZE 1024

void app_main(void) {
    // 1. Configurar UART para enviar la imagen
    uart_config_t uart_config = {
        .baud_rate = 2000000,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

    // 2. Iniciar Cámara
    if (init_camera() != ESP_OK) {
        printf("Fallo en la cámara\n");
        return;
    }

    printf("Camara lista. Enviando datos por VOT...\n");

    while (1) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) {
            // Enviar marcador de inicio para que el otro ESP sepa que viene una imagen
            uint8_t start_marker[] = {0xAA, 0xBB, 0xCC};
            uart_write_bytes(UART_NUM_0, (const char *)start_marker, 3);
            
            // Enviar el tamaño de la imagen (4 bytes)
            uart_write_bytes(UART_NUM_0, (const char *)&(fb->len), 4);
            
            // Enviar los datos del JPEG
            uart_write_bytes(UART_NUM_0, (const char *)fb->buf, fb->len);
            
            esp_camera_fb_return(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}