#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"

#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS   14
#define GPIO_DATAREADY 16

#define TOTAL_BYTES 64       // Size of each block
#define MAX_DATA_SIZE 700    // Total size of data to send

static QueueHandle_t spi_queue;

void IRAM_ATTR cs_interrupt_handler(void *arg);
void send_spi_data();

void configure_spi_slave() {
    printf("Initializing SPI Slave...\n");

    // SPI bus configuration
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TOTAL_BYTES,
    };

    // SPI slave configuration
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
    };

    // Initialize the SPI slave
    esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK) {
        printf("Error initializing SPI Slave: %d\n", ret);
        return;
    }
}

void IRAM_ATTR cs_interrupt_handler(void *arg) {
    int level = gpio_get_level(GPIO_CS);
    if (level == 0) {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(spi_queue, &level, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

void send_spi_data() {
    uint8_t sendbuf[TOTAL_BYTES];
    uint8_t recvbuf[TOTAL_BYTES];
    uint8_t block_num = 0;
    uint16_t total_bytes_sent = 0;

    while (1) {
        // Set Data Ready to 1
        gpio_set_level(GPIO_DATAREADY, 1);
        printf("Data Ready set to 1. Waiting for CS LOW...\n");

        int dummy;
        xQueueReceive(spi_queue, &dummy, portMAX_DELAY);
        gpio_set_level(GPIO_DATAREADY, 0);

        printf("CS LOW detected. Starting transmission...\n");

        while (total_bytes_sent < MAX_DATA_SIZE) {
            sendbuf[0] = 0x80 | (block_num & 0x7F);
            sendbuf[1] = block_num;
            memset(&sendbuf[2], 0xAA, TOTAL_BYTES - 2);

            memset(recvbuf, 0, sizeof(recvbuf));

            spi_slave_transaction_t t = {
                .length = 8 * TOTAL_BYTES,
                .tx_buffer = sendbuf,
                .rx_buffer = recvbuf
            };

            esp_err_t ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);
            if (ret == ESP_OK) {
                printf("Block %d sent successfully.\n", block_num);
            } else {
                printf("Error in SPI transaction\n");
            }

            total_bytes_sent += TOTAL_BYTES;
            block_num++;
        }

        printf("Waiting 5 seconds before next transmission...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void) {
    gpio_set_direction(GPIO_DATAREADY, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DATAREADY, 0);

    gpio_set_direction(GPIO_CS, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_CS, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_CS, cs_interrupt_handler, NULL);

    spi_queue = xQueueCreate(10, sizeof(int));
    configure_spi_slave();

    send_spi_data();
}