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
#include "spi_flash_mmap.h"
#include "driver/gpio.h"
#include "esp_mac.h"


// SPI pin configuration
#define GPIO_MOSI   23
#define GPIO_MISO   19
#define GPIO_SCLK   18
#define GPIO_CS      5

// SPI parameters
#define TOTAL_BYTES     1024  // 1KB per iteration

// Static buffer (DMA aligned 4 Bytes)
static uint8_t tx_buffer[TOTAL_BYTES] __attribute__((aligned(4)));

void init_buffer() {
    // First Byte: 0xFE (11111010)
    tx_buffer[0] = 0xFA;
    
    // General buffer: 0xA9 (10101001)
    for(int i = 1; i < TOTAL_BYTES; i++) {
        tx_buffer[i] = 0xA9;
    }
}

void app_main(void) {

    // 1. Initialize buffer
    init_buffer();

    // 2. SPI bus configuration
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TOTAL_BYTES,
    };

    // 3. SPI slave configuration
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,                  // SPI Mode 0 (CPOL=0, CPHA=0)
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0
    };

    // 4. SPI initialization
    ESP_ERROR_CHECK(spi_slave_initialize(SPI3_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO));

    // 5. Fixed transaction
    spi_slave_transaction_t t = {
        .length = TOTAL_BYTES * 8,  // Longitud fija (1024 bytes * 8 bits)
        .tx_buffer = tx_buffer,
        .rx_buffer = NULL
    };

    printf("SPI Slave iniciado. Patrón de datos:\n");
    printf("Primer byte: 0x%02X\n", tx_buffer[0]);
    printf("Bytes 2-1024: 0x%02X\n", tx_buffer[1]);

    // 6. Infinite transmission loop
    while(1) {
        // Waits for CS Low
        ESP_ERROR_CHECK(spi_slave_transmit(SPI3_HOST, &t, portMAX_DELAY));
        
        // Verificación rápida del buffer (opcional)
        // if(tx_buffer[0] != 0xFE) {  // Aseguramos primer byte
        //     tx_buffer[0] = 0xFE;
        // }
    }
}