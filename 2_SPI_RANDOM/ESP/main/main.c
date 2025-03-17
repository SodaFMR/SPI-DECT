#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"

#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS   14

#define TOTAL_BYTES 64       // Tamaño de cada bloque de datos
#define MAX_DATA_SIZE 700    // Total de datos a enviar

uint8_t complete_data[MAX_DATA_SIZE];  // Buffer con los 700 bytes de datos

// Función para inicializar el buffer de datos con valores aleatorios
void initialize_data() {
    for (int i = 0; i < MAX_DATA_SIZE; i++) {
        complete_data[i] = rand() % 256;  // Valores aleatorios de 0x00 a 0xFF
    }
}

// Función para enviar un bloque específico de datos
void send_block(uint8_t *sendbuf, uint8_t block_num, uint16_t offset, uint16_t length) {
    // Primer byte con el número de secuencia en los 7 bits menos significativos
    sendbuf[0] = 0x80 | (block_num & 0x7F);
    
    // Segundo byte con el número de bloque
    sendbuf[1] = block_num;

    // Copiar los datos desde `complete_data` al buffer de transmisión
    memcpy(&sendbuf[2], &complete_data[offset], length);

    // Rellenar con ceros si el último bloque tiene menos de 64 bytes
    if (length < TOTAL_BYTES - 2) {
        memset(&sendbuf[2 + length], 0, (TOTAL_BYTES - 2 - length));
    }
}

// Función principal
void app_main(void) {
    printf("Inicializando SPI Slave...\n");

    // Configuración del bus SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TOTAL_BYTES,
    };

    // Configuración del SPI esclavo
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
    };

    // Configurar el pin CS como entrada
    gpio_set_direction(GPIO_CS, GPIO_MODE_INPUT);

    // Inicializar SPI
    esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK) {
        printf("Error al inicializar SPI Slave: %d\n", ret);
        return;
    }

    // Inicializar los datos a enviar
    initialize_data();

    uint8_t sendbuf[TOTAL_BYTES];  // Buffer de envío
    uint8_t recvbuf[TOTAL_BYTES];  // Buffer de recepción

    uint8_t block_num = 0;  // Número del bloque actual
    uint16_t total_bytes_sent = 0;

    printf("Esperando transacción SPI...\n");

    while (1) {
        // Esperar a que CS esté en bajo
        while (gpio_get_level(GPIO_CS) == 1) {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }

        // Enviar bloques de 64 bytes hasta completar 700 bytes
        while (total_bytes_sent < MAX_DATA_SIZE) {
            uint16_t remaining_bytes = MAX_DATA_SIZE - total_bytes_sent;
            uint16_t bytes_to_send = (remaining_bytes >= TOTAL_BYTES - 2) ? (TOTAL_BYTES - 2) : remaining_bytes;

            // Preparar el bloque de datos a enviar
            send_block(sendbuf, block_num, total_bytes_sent, bytes_to_send);

            // Limpiar el buffer de recepción
            memset(recvbuf, 0, sizeof(recvbuf));

            spi_slave_transaction_t t = {0};
            t.length = 8 * TOTAL_BYTES;
            t.tx_buffer = sendbuf;
            t.rx_buffer = recvbuf;

            // Enviar el bloque
            printf("Enviando bloque %d (%d bytes)...\n", block_num, bytes_to_send + 2);
            ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);
            if (ret != ESP_OK) {
                printf("Error en la transacción SPI\n");
                return;
            }

            // Actualizar el estado de la transmisión
            total_bytes_sent += bytes_to_send;
            block_num++;

            vTaskDelay(100 / portTICK_PERIOD_MS);  // Retardo entre bloques
        }

        printf("Transmisión completada.\n");
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Esperar antes de reiniciar
    }
}
