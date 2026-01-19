#include "ili9488.h"
#include "font8x8.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>

static spi_device_handle_t _spi;

/*
🔹 ¿Qué es _spi?

Handle del dispositivo SPI (TFT)

Devuelto por spi_bus_add_device()

Usado internamente por todo el driver

🔹 ¿Por qué static?

Visible solo dentro de ili9488.c

Evita acceso externo indebido

Encapsulación correcta del driver

🧠 Modelo mental:

app_main → inicializa

ili9488_init() → guarda _spi

El resto del driver usa _spi sin pasarlo como parámetro
*/


void ili9488_send_cmd(uint8_t cmd) {
    gpio_set_level(PIN_NUM_DC, 0);
    spi_transaction_t t = {.length = 8, .tx_buffer = &cmd};
    spi_device_transmit(_spi, &t);
}

/*
🎯 Propósito

Enviar un comando al controlador ILI9488 a través de SPI.

En los TFT:

Comando → configura el controlador

Dato → píxeles o parámetros

La diferencia la marca el pin DC.
DC = 0 → COMANDO
*/

void ili9488_send_data(uint8_t data) {
    gpio_set_level(PIN_NUM_DC, 1);
    spi_transaction_t t = {.length = 8, .tx_buffer = &data};
    spi_device_transmit(_spi, &t);
}
/*
Envía un byte de datos al ILI9488 por SPI.
DC = 1 → lo que se envía es dato, no comando.
Se prepara una transacción SPI de 1 byte.
Se envía el byte al display (bloqueante).

Idea clave

send_cmd() → configura el display
send_data() → envía parámetros o píxeles
*/


void ili9488_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ili9488_send_cmd(0x2A);
    ili9488_send_data(x0 >> 8); ili9488_send_data(x0 & 0xFF);
    ili9488_send_data(x1 >> 8); ili9488_send_data(x1 & 0xFF);
    ili9488_send_cmd(0x2B);
    ili9488_send_data(y0 >> 8); ili9488_send_data(y0 & 0xFF);
    ili9488_send_data(y1 >> 8); ili9488_send_data(y1 & 0xFF);
    ili9488_send_cmd(0x2C);
}
/*
Define un rectángulo de la pantalla y deja al display listo para recibir píxeles solo en esa zona.
*/

void ili9488_init(spi_device_handle_t spi_dev) {
    _spi = spi_dev;

    // --- ESTO ES LO QUE DEBE ESTAR AQUÍ PARA QUE FUNCIONE SIEMPRE ---
    gpio_reset_pin(PIN_NUM_DC);
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT); // GPIO 2
    gpio_reset_pin(PIN_NUM_RST);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT); // GPIO 4
    // ----------------------------------------------------------------

    // Reset por hardware
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    ili9488_send_cmd(0x11); // Sleep OUT
    vTaskDelay(pdMS_TO_TICKS(120));

    ili9488_send_cmd(0x3A); 
    ili9488_send_data(0x66); // 18-bit mode

    ili9488_send_cmd(0x36); 
    ili9488_send_data(0x48);

    ili9488_send_cmd(0x29); // Display ON
    vTaskDelay(pdMS_TO_TICKS(100));
}

//Resetea el ILI9488, lo configura y lo deja listo para recibir imágenes.

void ili9488_draw_image(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data) {
    ili9488_set_window(x, y, x + w - 1, y + h - 1);
    gpio_set_level(PIN_NUM_DC, 1);
    spi_transaction_t t = {
        .length = w * h * 3 * 8, 
        .tx_buffer = data
    };
    spi_device_transmit(_spi, &t);
}
/*
Dibuja una imagen completa en la pantalla, en una zona específica.
Envía un frame (imagen) completo al display dentro de un rectángulo.
*/

void ili9488_fill_color(uint8_t r, uint8_t g, uint8_t b) {
    ili9488_set_window(0, 0, ILI9488_WIDTH - 1, ILI9488_HEIGHT - 1);
    uint8_t *buf = malloc(ILI9488_WIDTH * 3);
    if (!buf) return;
    for(int i = 0; i < ILI9488_WIDTH; i++) {
        buf[i*3] = r; buf[i*3+1] = g; buf[i*3+2] = b;
    }
    gpio_set_level(PIN_NUM_DC, 1);
    for(int i = 0; i < ILI9488_HEIGHT; i++) {
        spi_transaction_t t = {.length = ILI9488_WIDTH * 3 * 8, .tx_buffer = buf};
        spi_device_transmit(_spi, &t);
    }
    free(buf);
}

//rellena toda la pantalla con un solo color

void ili9488_draw_char(uint16_t x, uint16_t y, char c, uint8_t size, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t char_idx = (uint8_t)c;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (font8x8_basic[char_idx][i] & (1 << j)) {
                ili9488_set_window(x + j*size, y + i*size, x + (j+1)*size - 1, y + (i+1)*size - 1);
                for(int p = 0; p < size*size; p++) {
                    ili9488_send_data(r); ili9488_send_data(g); ili9488_send_data(b);
                }
            }
        }
    }
}

//Dibuja un carácter en pantalla usando la fuente 8×8, con color y escala.
//Convierte una letra 8×8 en píxeles de color sobre la pantalla, con escala.