// Slave as a transmitter for SPI communitation

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

int i; // Counter

// Main application
void app_main(void)
{
    // Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num=GPIO_MOSI,
        .miso_io_num=GPIO_MISO,
        .sclk_io_num=GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg={
        .mode=0,
        .spics_io_num=GPIO_CS,
        .queue_size=3,
        .flags=0,
    };

    // Initialize SPI slave interface
    spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);

    // SPI variables 
    char sendbuf[128] = {0};
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));
    
    printf("Slave transmission:\n");
    while (1)
    {
        snprintf(sendbuf, sizeof(sendbuf), "Sent by slave - %d", i);
        t.length = sizeof(sendbuf) * 8;
        t.tx_buffer = sendbuf;
        spi_slave_transmit(HSPI_HOST, &t, portMAX_DELAY);
        printf("Transmitted: %s\n", sendbuf);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;
    }
}