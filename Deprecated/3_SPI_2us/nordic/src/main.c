#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#define SPI_DEV_NODE DT_NODELABEL(spi3)
#define SPI_CS_NODE DT_NODELABEL(gpio0)
#define SPI_CS_PIN  7

static const struct device *spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
static const struct device *gpio_dev = DEVICE_DT_GET(SPI_CS_NODE);

struct spi_config spi_cfg = {
    .frequency = 32000000,
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_HOLD_ON_CS,
    .slave = 0,
};

#define TOTAL_BYTES 65
#define MAX_DATA_SIZE 700

static uint8_t recvbuf[TOTAL_BYTES] = {0};
static uint8_t data_buffer[MAX_DATA_SIZE] = {0};
static uint16_t total_bytes_received = 0;

void main(void)
{
    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

    struct spi_buf rx_buf = {
        .buf = recvbuf,
        .len = TOTAL_BYTES
    };
    struct spi_buf_set rx_buf_set = {
        .buffers = &rx_buf,
        .count = 1
    };

    printk("Initializing SPI in nRF9161 as Master...\n");

    if (!device_is_ready(spi_dev)) {
        printk("Error: SPI3 is not ready.\n");
        return;
    }
    if (!device_is_ready(gpio_dev)) {
        printk("Error: GPIO is not ready.\n");
        return;
    }

    while (total_bytes_received < MAX_DATA_SIZE) {
        memset(recvbuf, 0, TOTAL_BYTES);

        gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);
        int err = spi_transceive(spi_dev, &spi_cfg, NULL, &rx_buf_set);
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

        if (err) {
            printk("Error in spi_read(), failed to receive: %d\n", err);
        } else {
            uint8_t seq_num = recvbuf[0] & 0x7F;
            printk("Received block with sequence number: %d\n", seq_num);

            uint16_t bytes_to_copy = TOTAL_BYTES - 1;
            memcpy(&data_buffer[total_bytes_received], &recvbuf[1], bytes_to_copy);
            total_bytes_received += bytes_to_copy;

            printk("Received data (block %d):\n", seq_num);
            for (int i = 0; i < bytes_to_copy; i++) {
                printk("%02X ", recvbuf[i + 1]);
                if ((i + 1) % 16 == 0) {
                    printk("\n");
                }
            }
            printk("\n");

            if (total_bytes_received >= MAX_DATA_SIZE) {
                printk("All data received: %d bytes\n", total_bytes_received);
            }
        }

        k_busy_wait(2);
    }
}