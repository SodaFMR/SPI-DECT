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

static const struct spi_config spi_cfg = {
    .frequency = 7500000,   // 7.5 MHz
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .slave = 0,
};

#define TOTAL_BYTES 64

static uint8_t recvbuf[TOTAL_BYTES] = {0};

void main(void) 
{
    
    // Configure the CS pin as output and ensuring it is innactive
    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

    const struct spi_buf rx_buf = {
        .buf = recvbuf,
        .len = TOTAL_BYTES
    };
    const struct spi_buf_set rx_buf_set = {
        .buffers = &rx_buf,
        .count = 1
    };

    // Here the transmission buffer could be configured if needed
/*    
    struct spi_buf tx_buf = {
        .buf = sendbuf,
        .len = TOTAL_BYTES
    };
    struct spi_buf_set tx_buf_set = {
        .buffers = &tx_buf,
        .count = 1
    };
*/

    // Check the devices to be ready
    if (!device_is_ready(spi_dev)) {
        printk("Error: SPI3 is not ready.\n");
        return;
    }
    if (!device_is_ready(gpio_dev)) {
        printk("Error: GPIO is not ready.\n");
        return;
    }

    while (1) {
        // k_poll_signal_reset(&spi_done_sig);

        // Clean the buffer on each iteration
        memset(recvbuf, 0, TOTAL_BYTES);

        // CS is set to low to start receiving
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);
        
        int err = spi_transceive(spi_dev, &spi_cfg, NULL, &rx_buf_set);

        // CS is set to high on the end of the communication
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

        if (err) {
            printk("Error: %d\n", err);
        } else
        {
            printk("Received %d bytes\n", TOTAL_BYTES);
            // for (int i = 0; i < TOTAL_BYTES; i++) {
            //     printk("Byte %d: 0x%02X\n", i, recvbuf[i]);
            // }
        }
        k_msleep(500);
    }
}