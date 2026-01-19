#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "jpeg_decoder.h"
#include "ili9488.h"
#include "jpg_decode.h"

static const char *TAG = "JPG_DEC";

esp_err_t draw_jpg_frame(uint8_t *jpeg_data, uint32_t size) {
    if (jpeg_data == NULL || size == 0) return ESP_FAIL;

    esp_jpeg_image_output_t out_img_info;
    esp_jpeg_image_cfg_t cfg = {
        .indata = jpeg_data,
        .indata_size = size,
        .out_format = JPEG_IMAGE_FORMAT_RGB888, 
        .out_scale = JPEG_IMAGE_SCALE_0,        
        .flags = {
            .swap_color_bytes = 0, 
        }
    };

    esp_err_t ret = esp_jpeg_get_image_info(&cfg, &out_img_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al obtener info del JPEG");
        return ret;
    }

    // Reservar memoria
    uint8_t *out_buffer = malloc(out_img_info.output_len);
    if (out_buffer == NULL) {
        cfg.out_scale = JPEG_IMAGE_SCALE_1_2;
        esp_jpeg_get_image_info(&cfg, &out_img_info);
        out_buffer = malloc(out_img_info.output_len);
        if (out_buffer == NULL) return ESP_ERR_NO_MEM;
    }

    cfg.outbuf = out_buffer;
    cfg.outbuf_size = out_img_info.output_len;

    ret = esp_jpeg_decode(&cfg, &out_img_info);
    if (ret == ESP_OK) {
        // --- AJUSTE DE POSICIÓN ---
        // Definimos el tamaño de TU pantalla (480x320)
        const uint16_t SCREEN_W = 480;
        const uint16_t SCREEN_H = 320;

        // Calculamos coordenadas para que la imagen quede centrada
        uint16_t x_offset = (out_img_info.width < SCREEN_W) ? (SCREEN_W - out_img_info.width) / 2 : 0;
        uint16_t y_offset = (out_img_info.height < SCREEN_H) ? (SCREEN_H - out_img_info.height) / 2 : 0;

        // Dibujar en la posición calculada
        ili9488_draw_image(x_offset, y_offset, out_img_info.width, out_img_info.height, out_buffer);
    } else {
        ESP_LOGE(TAG, "Fallo en decodificacion: %d", ret);
    }

    free(out_buffer);
    return ret;
}