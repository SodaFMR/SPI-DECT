#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include <stdio.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

#define SPI_HOST        VSPI_HOST    // Usamos el SPI HSPI
#define PIN_NUM_MISO    19           // Pin MISO
#define PIN_NUM_CLK     18           // Pin CLK
#define PIN_NUM_MOSI    23           // Pin MOSI
#define PIN_NUM_CS      5            // Pin CS (Chip Select)

void app_main() {
    esp_err_t ret;
    uint8_t data_to_send = 0xA5; // Dato hexadecimal a enviar (ejemplo: 0xA5)

    spi_bus_config_t bus_cfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 1000000,      // 1 MHz
        .mode = 0,                      // Modo SPI 0
        .spics_io_num = PIN_NUM_CS,     // CS en el pin 5
        .queue_size = 1,                 // Solo una transacción
        .flags = SPI_DEVICE_HALFDUPLEX  // Solo recepción en el maestro
    };

    // Inicializar el bus SPI
    ret = spi_bus_initialize(SPI_HOST, &bus_cfg, 1);
    if (ret != ESP_OK) {
        ESP_LOGE("SPI", "Failed to initialize SPI bus");
        return;
    }

    // Agregar dispositivo SPI al bus
    spi_device_handle_t handle;
    ret = spi_bus_add_device(SPI_HOST, &dev_cfg, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE("SPI", "Failed to add SPI device");
        return;
    }

    // Configurar el pin CS como entrada (habilitado en LOW)
    gpio_set_direction(PIN_NUM_CS, GPIO_MODE_INPUT);

    // Loop para enviar datos al maestro
    while (1) {
        // Configurar el buffer de transmisión
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));
        t.length = 8 * sizeof(data_to_send);  // 8 bits
        t.tx_buffer = &data_to_send;          // Dato a enviar
        t.rx_buffer = NULL;                   // No necesitamos recibir datos

        // Iniciar la transacción SPI
        ret = spi_device_transmit(handle, &t);
        if (ret != ESP_OK) {
            ESP_LOGE("SPI", "Transmission failed");
        }
    }
}
