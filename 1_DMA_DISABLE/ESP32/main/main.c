#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "esp_task_wdt.h"

#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS   14
#define GPIO_DATAREADY 16

#define TOTAL_BYTES 64       // Tamaño de cada bloque
#define MAX_DATA_SIZE 700    // Tamaño total de los datos a enviar

static QueueHandle_t cs_queue;  // Cola para manejar la interrupción de CS
static bool spi_busy = false;   // Bandera para controlar la concurrencia

// Función para llenar un bloque con la secuencia deseada
void send_block(uint8_t *sendbuf, uint8_t block_num)
{
    sendbuf[0] = 0x80 | (block_num & 0x7F);
    sendbuf[1] = block_num;

    uint8_t sequence[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    int sequence_len = sizeof(sequence) / sizeof(sequence[0]);

    for (int j = 2; j < TOTAL_BYTES; j++) {
        sendbuf[j] = sequence[(j - 2) % sequence_len];
    }
}

// **Rutina de interrupción para detectar CS en LOW**
static void IRAM_ATTR cs_isr_handler(void *arg)
{
    int cs_level = gpio_get_level(GPIO_CS);
    if (cs_level == 0 && !spi_busy) {  // Solo iniciar transacciones si no está ocupado SPI
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (uxQueueSpacesAvailable(cs_queue) > 0) {
            xQueueSendFromISR(cs_queue, &cs_level, &xHigherPriorityTaskWoken);
        }
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// **Tarea que maneja la comunicación SPI cuando CS pasa a LOW**
void spi_slave_task(void *arg)
{
    uint8_t sendbuf[TOTAL_BYTES];
    uint8_t recvbuf[TOTAL_BYTES];
    uint8_t block_num = 0;
    uint16_t total_bytes_sent = 0;
    int cs_signal;
    esp_err_t ret;

    while (1)
    {
        if (xQueueReceive(cs_queue, &cs_signal, portMAX_DELAY))
        {
            spi_busy = true;  // Marca SPI como ocupado
            while (total_bytes_sent < MAX_DATA_SIZE)
            {
                send_block(sendbuf, block_num);
                memset(recvbuf, 0, sizeof(recvbuf));

                spi_slave_transaction_t t = {
                    .length = 8 * TOTAL_BYTES,
                    .tx_buffer = sendbuf,
                    .rx_buffer = recvbuf
                };

                ret = spi_slave_transmit(SPI2_HOST, &t, pdMS_TO_TICKS(1000)); // Timeout de 1 segundo
                if (ret != ESP_OK) {
                    printf("Error en la transacción SPI\n");
                    break; // Evita bloqueo infinito
                }

                total_bytes_sent += TOTAL_BYTES;
                block_num++;
                vTaskDelay(pdMS_TO_TICKS(10)); // Pequeño retardo para evitar saturación
            }

            uint16_t remaining_bytes = MAX_DATA_SIZE % TOTAL_BYTES;
            if (remaining_bytes > 0) {
                printf("Enviando %d bytes residuales...\n", remaining_bytes);

                send_block(sendbuf, block_num);
                memset(&sendbuf[remaining_bytes], 0, TOTAL_BYTES - remaining_bytes);
                memset(recvbuf, 0, sizeof(recvbuf));

                spi_slave_transaction_t t = {
                    .length = 8 * remaining_bytes,
                    .tx_buffer = sendbuf,
                    .rx_buffer = recvbuf
                };

                ret = spi_slave_transmit(SPI2_HOST, &t, pdMS_TO_TICKS(1000));
                if (ret == ESP_OK) {
                    printf("Bytes residuales enviados correctamente.\n");
                } else {
                    printf("Error en la transacción SPI\n");
                }

                block_num++;
            }

            // **Desactivar DataReady después de la transmisión**
            gpio_set_level(GPIO_DATAREADY, 0);
            total_bytes_sent = 0;
            spi_busy = false;  // Marca SPI como libre
        }
    }
}

void app_main(void)
{
    // Deshabilitar el watchdog de FreeRTOS
    esp_task_wdt_deinit();

    printf("Inicializando SPI Esclavo con interrupción en CS...\n");

    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = TOTAL_BYTES
    };

    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0
    };

    esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_DISABLED);
    if (ret != ESP_OK) {
        printf("Error al inicializar SPI Esclavo: %d\n", ret);
        return;
    }

    gpio_set_direction(GPIO_CS, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    gpio_set_direction(GPIO_DATAREADY, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DATAREADY, 0);

    cs_queue = xQueueCreate(10, sizeof(int));

    gpio_install_isr_service(0);
    gpio_set_intr_type(GPIO_CS, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(GPIO_CS, cs_isr_handler, NULL);

    xTaskCreate(spi_slave_task, "spi_slave_task", 4096, NULL, 10, NULL);

    while (1) {
        printf("Esperando para activar DataReady...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));

        printf("Activando DataReady = 1\n");
        gpio_set_level(GPIO_DATAREADY, 1);
    }
}
