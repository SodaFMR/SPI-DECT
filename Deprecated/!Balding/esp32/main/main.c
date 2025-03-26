#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/igmp.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "soc/rtc_periph.h"
#include "driver/spi_slave.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"

// Pins in use
#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

uint8_t i = 0; // Counter

// Main application
void app_main(void)
{
    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
    };

    // Initialize SPI slave interface
    spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);

    // Variables SPI
    uint8_t sendbuf[2];
    uint8_t recvbuf[2];

    printf("Slave transmission:\n");

    while (1)
    {
        // Configurar el mensaje a enviar
        sendbuf[0] = i;     // Counter
        sendbuf[1] = 0xAC;

        // Limpiar la estructura de transacción
        spi_slave_transaction_t t;
        memset(&t, 0, sizeof(t));

        // Configurar buffers de transmisión y recepción
        t.length = sizeof(sendbuf) * 8;  // Longitud en bits
        t.tx_buffer = sendbuf;
        t.rx_buffer = recvbuf;

        // Esperar y realizar la transacción SPI
        ESP_ERROR_CHECK_WITHOUT_ABORT(spi_slave_transmit(HSPI_HOST, &t, portMAX_DELAY));

        // Imprimir datos enviados por el esclavo
        printf("Tx: %02x %02x\n", sendbuf[0], sendbuf[1]);

        // Imprimir datos recibidos del maestro
        printf("Rx: %02x %02x\n", recvbuf[0], recvbuf[1]);

        i++;
        
        // Esperar 1 segundo antes de la siguiente transmisión
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}