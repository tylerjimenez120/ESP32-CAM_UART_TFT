#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "ili9488.h"
#include "receiver_logic.h"
#include "jpg_decode.h"  // <-- INCLUIMOS TU NUEVO MÓDULO

#define BUF_SIZE 40000

void app_main(void) {
    printf("[BITACORA] Iniciando Receptor v1.0...\n");

    // 1. Configuración SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = 23,
        .miso_io_num = -1,
        .sclk_io_num = 18,
        .max_transfer_sz = BUF_SIZE * 3
    };
    spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS, // GPIO 5
        .queue_size = 7
    };
    spi_device_handle_t spi;
    spi_bus_add_device(HSPI_HOST, &devcfg, &spi);

    // 2. Inicializar Pantalla
    ili9488_init(spi);
    ili9488_fill_color(0, 0, 0); 

    // Confirmación visual
    ili9488_draw_char(10, 10, 'O', 2, 0, 255, 0);
    ili9488_draw_char(30, 10, 'K', 2, 0, 255, 0);

    // 3. Inicializar UART para Cámara
    init_uart_receiver();

    uint8_t *img_data = malloc(BUF_SIZE);
    uint32_t img_size;

    if (img_data == NULL) {
        printf("[ERROR] No hay memoria para img_data\n");
        return;
    }

    printf("[BITACORA] Esperando flujo de video (Streaming)...\n");

    

    while (1) {
        // wait_for_frame llena img_data y nos da el img_size
        if (wait_for_frame(img_data, &img_size) == ESP_OK) {
            
            // --- NUEVO: PROCESAR STREAMING ---
            // Decodifica el JPEG y lo manda directo a la ILI9488
            if (draw_jpg_frame(img_data, img_size) == ESP_OK) {
                // Opcional: imprimir en consola para monitorear
                printf("[BITACORA] Frame renderizado: %ld bytes\n", img_size);
            } else {
                printf("[BITACORA] Error al decodificar imagen\n");
            }
        }
        
        // Un delay pequeño para que el Watchdog del sistema no se active
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
 

