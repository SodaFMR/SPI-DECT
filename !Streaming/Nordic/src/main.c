#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#define SPI_DEV_NODE DT_NODELABEL(spi3)
#define SPI_CS_NODE DT_NODELABEL(gpio0)
#define SPI_CS_PIN  7

#define TRANSFER_SIZE 64
#define BLOCKS_PER_BUFFER 16
#define BUFFER_SIZE (TRANSFER_SIZE * BLOCKS_PER_BUFFER)

static const struct device *spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
static const struct device *gpio_dev = DEVICE_DT_GET(SPI_CS_NODE);

static uint8_t rx_buffer[TRANSFER_SIZE];
static uint8_t full_buffer[BUFFER_SIZE];
static size_t total_received = 0;
static size_t crc_error_count = 0;

static const struct spi_config spi_cfg = {
    .frequency = 7500000,
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .slave = 0,
};

uint8_t calculate_crc(uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len - 1; i++) {
        crc ^= data[i];
    }
    return crc;
}

void main(void) {
    const struct spi_buf rx_buf = {
        .buf = rx_buffer,
        .len = TRANSFER_SIZE
    };
    const struct spi_buf_set rx_buf_set = {
        .buffers = &rx_buf,
        .count = 1
    };

    if (!device_is_ready(spi_dev)) {
        printk("Error: SPI device not ready.\n");
        return;
    }
    if (!device_is_ready(gpio_dev)) {
        printk("Error: GPIO device not ready.\n");
        return;
    }

    // Configurar pin CS como salida y desactivado
    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

    printk("Iniciando recepción SPI manual con control de CS...\n");

    while (1) {
        memset(rx_buffer, 0, TRANSFER_SIZE);

        gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);  // CS LOW
        int ret = spi_transceive(spi_dev, &spi_cfg, NULL, &rx_buf_set);
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);  // CS HIGH

        if (ret != 0) {
            printk("Error en spi_transceive: %d\n", ret);
            continue;
        }

        uint8_t expected_crc = rx_buffer[TRANSFER_SIZE - 1];
        uint8_t calculated_crc = calculate_crc(rx_buffer, TRANSFER_SIZE);

        if (expected_crc != calculated_crc) {
            crc_error_count++;
        }

        memcpy(&full_buffer[total_received], rx_buffer, TRANSFER_SIZE);
        total_received += TRANSFER_SIZE;

        if (total_received >= BUFFER_SIZE) {
            printk("Recibidos %d bytes. Errores de CRC acumulados: %d\n", BUFFER_SIZE, crc_error_count);
            total_received = 0;
        }

        // k_msleep(500); // Puede ajustarse o eliminarse si se desea máxima velocidad
    }
}