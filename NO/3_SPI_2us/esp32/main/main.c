#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS   14

#define TOTAL_BYTES 64       // Size of each data block
#define MAX_DATA_SIZE 700    // Total data to send

uint8_t complete_data[MAX_DATA_SIZE];  // Buffer containing 700 bytes of data

// Function to initialize the data buffer with random values
void initialize_data() {
    for (int i = 0; i < MAX_DATA_SIZE; i++) {
        complete_data[i] = rand() % 256;  // Random values from 0x00 to 0xFF
    }
}

// Function to prepare a specific data block for transmission
void prepare_block(uint8_t *sendbuf, uint8_t block_num, uint16_t offset, uint16_t length) {
    sendbuf[0] = 0x80 | (block_num & 0x7F);  // Header byte with sequence number
    sendbuf[1] = block_num;  // Block number

    memcpy(&sendbuf[2], &complete_data[offset], length);  // Copy data to the buffer

    if (length < TOTAL_BYTES - 2) {
        memset(&sendbuf[2 + length], 0, (TOTAL_BYTES - 2 - length));  // Fill with zeros if necessary
    }
}

// Function to execute SPI transmission
void execute_spi_transmission() {
    uint8_t sendbuf[TOTAL_BYTES];  // Transmission buffer
    uint8_t recvbuf[TOTAL_BYTES];  // Reception buffer
    uint8_t block_num = 0;  // Current block number
    uint16_t total_bytes_sent = 0;

    printf("Waiting for SPI transaction...\n");

    while (1) {
        // Wait for CS to go low (indicates that the master is ready to receive)
        while (gpio_get_level(GPIO_CS) == 1) {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }

        // Send 64-byte blocks until 700 bytes are transmitted
        while (total_bytes_sent < MAX_DATA_SIZE) {
            uint16_t remaining_bytes = MAX_DATA_SIZE - total_bytes_sent;
            uint16_t bytes_to_send = (remaining_bytes >= TOTAL_BYTES - 2) ? (TOTAL_BYTES - 2) : remaining_bytes;

            // Prepare the data block to send
            prepare_block(sendbuf, block_num, total_bytes_sent, bytes_to_send);

            // Clear the reception buffer
            memset(recvbuf, 0, sizeof(recvbuf));

            spi_slave_transaction_t t = {0};
            t.length = 8 * TOTAL_BYTES;  // Send 64 bytes (512 bits)
            t.tx_buffer = sendbuf;
            t.rx_buffer = recvbuf;

            // Send the block
            esp_err_t ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);
            if (ret != ESP_OK) {
                printf("SPI transaction error\n");
                return;
            }

            // Update transmission status
            total_bytes_sent += bytes_to_send;
            block_num++;

            // Wait 2 microseconds before sending the next block
            ets_delay_us(2);
        }

        printf("Transmission completed.\n");

        // Reset for next transmission
        total_bytes_sent = 0;
        block_num = 0;
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void app_main(void) {
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

    // SPI slave configuration without DMA
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
    };

    // Set CS pin as input
    gpio_set_direction(GPIO_CS, GPIO_MODE_INPUT);

    // Initialize SPI without DMA
    esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK) {
        printf("Error initializing SPI Slave: %d\n", ret);
        return;
    }

    initialize_data();  // Fill the buffer with random data

    // Execute SPI transmission
    execute_spi_transmission();
}
