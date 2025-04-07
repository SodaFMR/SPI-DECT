#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "esp_task_wdt.h"  // Incluido para el WDT

#define GPIO_MOSI 23
#define GPIO_MISO 19
#define GPIO_SCLK 18
#define GPIO_CS   5
#define GPIO_DATAREADY 16

<<<<<<< HEAD
#define TOTAL_BYTES 64
#define HEADER_SIZE 1
#define CRC_SIZE 2
#define PAYLOAD_SIZE (TOTAL_BYTES - HEADER_SIZE - CRC_SIZE)
#define MAX_DATA_SIZE 4000
=======
#define TOTAL_BYTES 64       // Size of each block
#define MAX_DATA_SIZE 4000    // Total size of data to send
>>>>>>> 486ca3ed37f137a260fb34ffaa5f4beddc757a65

static const char *TAG = "SPI_Slave";

// CRC-16 (Modbus)
uint16_t calculate_crc16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

void prepare_block(uint8_t *sendbuf, uint8_t block_num) {
    // Header: 1xxx xxxx (bit7=1, bits6-0=block_num)
    sendbuf[0] = 0x80 | (block_num & 0x7F);

    // Payload fijo (podría ser datos reales)
    uint8_t sequence[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    for (int j = 1; j < TOTAL_BYTES - CRC_SIZE; j++) {
        sendbuf[j] = sequence[(j-1) % (sizeof(sequence)/sizeof(sequence[0]))];
    }

    // CRC (header + payload)
    uint16_t crc = calculate_crc16(sendbuf, TOTAL_BYTES - CRC_SIZE);
    sendbuf[TOTAL_BYTES-2] = crc >> 8;    // MSB
    sendbuf[TOTAL_BYTES-1] = crc & 0xFF;  // LSB
}

// ISR para flanco de bajada de CS
static void IRAM_ATTR cs_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR((TaskHandle_t)arg, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

void spi_task(void *arg) {
    uint8_t sendbuf[TOTAL_BYTES];
    uint8_t block_num = 0;
    int bytes_remaining = MAX_DATA_SIZE;

    while (1) {
        // 1. Esperar activación por CS (desde ISR)
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // 2. Preparar datos (antes de activar DataReady)
        prepare_block(sendbuf, block_num);

        // 3. Señalizar datos listos
        gpio_set_level(GPIO_DATAREADY, 1);

        // 4. Configurar transacción SPI
        spi_slave_transaction_t t = {
            .length = 8 * TOTAL_BYTES,
            .tx_buffer = sendbuf,
            .rx_buffer = NULL
        };

        // 5. Transmitir (se bloquea hasta que el maestro baje CS)
        esp_err_t ret = spi_slave_transmit(SPI3_HOST, &t, portMAX_DELAY);
        
        // 6. Post-transmisión
        gpio_set_level(GPIO_DATAREADY, 0);
        
        if (ret == ESP_OK) {
            bytes_remaining -= PAYLOAD_SIZE;
            block_num = (bytes_remaining <= 0) ? 0 : block_num + 1;
            ESP_LOGD(TAG, "Block %d sent", block_num);
        } else {
            ESP_LOGE(TAG, "SPI error: %d", ret);
        }

        // Reset si se completó la transmisión
        if (bytes_remaining <= 0) {
            bytes_remaining = MAX_DATA_SIZE;
        }
    }
}

void app_main() {
    // Deshabilitar el watchdog como en tu primer ejemplo
    esp_task_wdt_deinit();
    
    ESP_LOGI(TAG, "SPI Slave initialization (WDT disabled)");

    // 1. Configuración GPIO
    gpio_config_t cs_conf = {
        .pin_bit_mask = (1ULL << GPIO_CS),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&cs_conf);

    gpio_set_direction(GPIO_DATAREADY, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DATAREADY, 0);

    // 2. Configuración SPI
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TOTAL_BYTES
    };

    spi_slave_interface_config_t slvcfg = {
        .mode = 0,  // SPI_MODE_0
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0
    };

    ESP_ERROR_CHECK(spi_slave_initialize(SPI3_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED));

    // 3. Tarea e interrupciones
    TaskHandle_t spi_task_handle;
    xTaskCreate(spi_task, "spi_task", 4096, NULL, 5, &spi_task_handle);

<<<<<<< HEAD
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_CS, cs_isr_handler, spi_task_handle);

    ESP_LOGI(TAG, "SPI Slave ready. Data format:");
    ESP_LOGI(TAG, "Header: 1xxx xxxx | Payload: 61 bytes | CRC-16: 2 bytes");
=======
    // Initialize the SPI slave
    esp_err_t ret = spi_slave_initialize(SPI3_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK) {
        printf("Error initializing SPI Slave: %d\n", ret);
        return;
    }

    uint8_t sendbuf[TOTAL_BYTES];
    uint8_t recvbuf[TOTAL_BYTES];
    uint8_t block_num = 0; // Block number to send
    int16_t bytes_to_send = MAX_DATA_SIZE;     // Will be used to check the residual

    printf("Waiting for SPI transaction...\n");

    while (1)
    {
        // Wait until CS is low
        while (gpio_get_level(GPIO_CS) == 1)
        {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }

        while (bytes_to_send > 0)
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

            gpio_set_level(GPIO_DATAREADY, 1);  // Set DataReady to 1 to signal that data is available for the master

            ret = spi_slave_transmit(SPI3_HOST, &t, portMAX_DELAY);
            if (ret == ESP_OK) {
                printf("Block %d sent successfully.\n", block_num);
            } else {
                printf("Error in SPI transaction\n");
            }

            bytes_to_send -= TOTAL_BYTES;
            printf("Bytes left to send: %d\n", bytes_to_send);
            block_num++;

            vTaskDelay(50 / portTICK_PERIOD_MS);
            // Here the program will have reached the residual data, leaving this while loop
        }

        // Send the residual bytes
        uint16_t remaining_bytes = MAX_DATA_SIZE % TOTAL_BYTES;
        if (remaining_bytes > 0) {
            printf("Sending %d remaining bytes...\n", remaining_bytes);

            send_block(sendbuf, 0xFF);
            memset(&sendbuf[remaining_bytes], 0, TOTAL_BYTES - remaining_bytes);

            // Print the residual bytes before sending
            printf("Residual bytes to send:\n");
            for (int i = 0; i < TOTAL_BYTES; i++) {
                printf("%02X ", sendbuf[i]);
                if ((i + 1) % 16 == 0) {
                    printf("\n");
                }
            }
            printf("- - - - - - - - - - - - - - - - - - - - - - - -\n");

            memset(recvbuf, 0, sizeof(recvbuf));

            spi_slave_transaction_t t;
            memset(&t, 0, sizeof(t));
            t.length = 8 * remaining_bytes;
            t.tx_buffer = sendbuf;
            t.rx_buffer = recvbuf;

            gpio_set_level(GPIO_DATAREADY, 0);  // Set DataReady to 0 after sending the data
            
            ret = spi_slave_transmit(SPI3_HOST, &t, portMAX_DELAY);
            if (ret == ESP_OK) {
                printf("Remaining bytes sent successfully.\n");
            } else {
                printf("Error in SPI transaction\n");
            }
            remaining_bytes = 0;
        }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
        break;  // Exit the loop after sending the residual data
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
>>>>>>> 486ca3ed37f137a260fb34ffaa5f4beddc757a65
}