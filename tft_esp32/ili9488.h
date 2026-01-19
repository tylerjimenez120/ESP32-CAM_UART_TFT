#ifndef ILI9488_H
#define ILI9488_H

#include "driver/spi_master.h"
#include <stdint.h>

#define ILI9488_WIDTH  320
#define ILI9488_HEIGHT 480

/*
Validar coordenadas
Calcular buffers
Escalar imagen de la cámara
*/

// Pines de control
#define PIN_NUM_DC   2 //data /command
#define PIN_NUM_RST  4 //reset del display
#define PIN_NUM_CS   5 //chip select spi

void ili9488_init(spi_device_handle_t spi_dev);
/*
configura el controlador spi
envia comandos de setup
prepara tft para reibir comandos
*/

void ili9488_fill_color(uint8_t r, uint8_t g, uint8_t b);
/*
Llena la pantalla con un color RGB
Usado al iniciar o cambiar de modo
*/

void ili9488_draw_char(uint16_t x, uint16_t y, char c, uint8_t size, uint8_t r, uint8_t g, uint8_t b);
/*
*/

void ili9488_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
/*
Define un área donde:
Se escriben píxeles consecutivos
Se envía un frame completo o parcial
*/

void ili9488_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data);
/*
👉 Aquí entra el frame de la ESP32-CAM:
RGB565 o RGB888
Buffer continuo
Usado para “streaming” simulado
*/

#endif