#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"

#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS   14
#define GPIO_DATAREADY 16

#define TOTAL_BYTES 64       // Size of each block
#define MAX_DATA_SIZE 700    // Total size of data to send

// Function to fill a block with the desired sequence
void send_block(uint8_t *sendbuf, uint8_t block_num)
{
    sendbuf[0] = 0x80 | (block_num & 0x7F);  // First byte for other blocks
    sendbuf[1] = block_num;  // Second byte with block number

    // Fixed sequence in bytes 2 to 63
    uint8_t sequence[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    int sequence_len = sizeof(sequence) / sizeof(sequence[0]);

    for (int j = 2; j < TOTAL_BYTES; j++)
    {
        sendbuf[j] = sequence[(j - 2) % sequence_len];  // Cycle through the sequence
    }
}

void app_main(void)
{
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

    // Set CS as input
    gpio_set_direction(GPIO_CS, GPIO_MODE_INPUT);

    // Initialize the SPI slave
    esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK) {
        printf("Error initializing SPI Slave: %d\n", ret);
        return;
    }

    uint8_t sendbuf[TOTAL_BYTES];
    uint8_t recvbuf[TOTAL_BYTES];
    uint8_t block_num = 0; // Block number to send
    uint16_t total_bytes_sent = 0;

    printf("Waiting for SPI transaction...\n");

    while (1)
    {
        // Wait until CS is low
        while (gpio_get_level(GPIO_CS) == 1)
        {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }

        while (total_bytes_sent < MAX_DATA_SIZE)
        {
            send_block(sendbuf, block_num);  // Fill the block

            memset(recvbuf, 0, sizeof(recvbuf));

            spi_slave_transaction_t t;
            memset(&t, 0, sizeof(t));
            t.length = 8 * TOTAL_BYTES;
            t.tx_buffer = sendbuf;
            t.rx_buffer = recvbuf;

            printf("Sending block %d: \n", block_num);
            for (int j = 0; j < TOTAL_BYTES; j++) {
                printf("%02X ", sendbuf[j]);
                if ((j + 1) % 16 == 0) {
                    printf("\n");
                }
            }
            printf("- - - - - - - - - - - - - - - - - - - - - - - -\n");

            ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);
            if (ret == ESP_OK) {
                printf("Block %d sent successfully.\n", block_num);
            } else {
                printf("Error in SPI transaction\n");
            }

            total_bytes_sent += TOTAL_BYTES;
            block_num++;

            vTaskDelay(500 / portTICK_PERIOD_MS);
        }

        uint16_t remaining_bytes = MAX_DATA_SIZE % TOTAL_BYTES;
        if (remaining_bytes > 0) {
            printf("Sending %d remaining bytes...\n", remaining_bytes);

            send_block(sendbuf, block_num);
            memset(&sendbuf[remaining_bytes], 0, TOTAL_BYTES - remaining_bytes);

            memset(recvbuf, 0, sizeof(recvbuf));

            spi_slave_transaction_t t;
            memset(&t, 0, sizeof(t));
            t.length = 8 * remaining_bytes;
            t.tx_buffer = sendbuf;
            t.rx_buffer = recvbuf;

            ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);
            if (ret == ESP_OK) {
                printf("Remaining bytes sent successfully.\n");
            } else {
                printf("Error in SPI transaction\n");
            }

            block_num++;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}