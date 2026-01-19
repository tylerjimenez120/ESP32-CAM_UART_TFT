#ifndef JPG_DECODE_H
#define JPG_DECODE_H

#include <stdint.h>
#include "esp_err.h"
#include "ili9488.h"

/**
 * @brief Decodifica un buffer JPEG y lo dibuja directamente en la pantalla.
 * * @param jpeg_data Buffer con los datos JPEG recibidos por UART.
 * @param size Tamaño del buffer JPEG.
 * @return esp_err_t ESP_OK si la decodificación y el dibujado fueron exitosos.
 */
esp_err_t draw_jpg_frame(uint8_t *jpeg_data, uint32_t size);

#endif