#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#define SPI_DEV_NODE DT_NODELABEL(spi3)

// GPIO pin for the CS, but it is actually sip3 in the overlay file
#define SPI_CS_NODE DT_NODELABEL(gpio0)
#define SPI_CS_PIN  7

// Both GPIO and SPI devices are found here
static const struct device *spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
static const struct device *gpio_dev = DEVICE_DT_GET(SPI_CS_NODE);

// Configuration for the SPI device
struct spi_config spi_cfg = {
    .frequency = 1000000,
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,   // Mode 0, CPOL=0, CPHA=0
    .slave = 0,
};

// Buffer size and buffer to store the received data
#define BUF_SIZE 2
static uint8_t recvbuf[BUF_SIZE] = {0};
static uint8_t sendbuf[BUF_SIZE] = {0xAA, 0xCA};

void main(void)
{
    // Here GPIO pin CS gets configured as output
    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);
    
    struct spi_buf tx_buf = {
        .buf = sendbuf,
        .len = BUF_SIZE
    };
    struct spi_buf_set tx_buf_set = {
        .buffers = &tx_buf,
        .count = 1
    };

    struct spi_buf rx_buf = {
        .buf = recvbuf,
        .len = BUF_SIZE
    };
    struct spi_buf_set rx_buf_set = {
        .buffers = &rx_buf,
        .count = 1
    };

    printk("Initializing SPI in nRF9161 as Master...\n");

    // Check all the modules used are available and ready to use
    if (!device_is_ready(spi_dev)) {
        printk("Error: SPI3 no está listo.\n");
        return;
    }
    if (!device_is_ready(gpio_dev)) {
        printk("Error: GPIO no está listo.\n");
        return;
    }

    // Begining of the spi functionality (receiving data)
    while (1) {
        // I decided to use memset to clean the buffer before each iteration
        // so as to avoid any garbage data that could be present in the buffer
        memset(recvbuf, 0, BUF_SIZE);

        // CS pin is set to low to start the SPI communication
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);
//       k_sleep(K_USEC(1));

        // Here the received data from spi is stored in the buffer (err checks for errors)
        int err = spi_transceive(spi_dev, &spi_cfg, &tx_buf_set, &rx_buf_set);
        
        // CS pin is set now to high to end the SPI communication
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

        if (err) {
            printk("Error in spi_read(), failed to receive: %d\n", err);
        } else {
            printk("Data received: %02X %02X\n", recvbuf[0], recvbuf[1]);
        }

        // Delay of 1 second before the next iteration
        k_msleep(1000);
    }
}
