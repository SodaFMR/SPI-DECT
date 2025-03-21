#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define SPI_DEV    "spi3"  // SPI device (same as in the master)
#define SPI_CS_PIN 20       // Chip Select (must match the master)
#define GPIO_DEV   "GPIO_0" // GPIO device used for CS pin
#define BUFFER_SIZE 1       // Buffer size for receiving data (1 byte)

/* SPI Slave configuration */
static const struct spi_config spi_cfg = {
    .operation = SPI_OP_MODE_SLAVE | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .frequency = 8000000,   // Same frequency as master (8 MHz)
    .slave = 1,             // Slave mode
//    .cs = NULL,             // Master controls CS manually
};

void main(void)
{
    const struct device *spi_dev;
    const struct device *gpio_dev;
    
    uint8_t rx_buffer[BUFFER_SIZE] = {0x00};  // Reception buffer
    struct spi_buf rx_buf = {
        .buf = rx_buffer,
        .len = sizeof(rx_buffer),
    };
    struct spi_buf_set rx = {
        .buffers = &rx_buf,
        .count = 1,
    };

    /* Get the SPI device */
    spi_dev = device_get_binding(SPI_DEV);
    if (!spi_dev) {
        printk("Error: No SPIS device found.\n");
        return;
    }

    /* Get the GPIO device to manage the CS pin */
    gpio_dev = device_get_binding(GPIO_DEV);
    if (!gpio_dev) {
        printk("Error: No GPIO device found on the slave.\n");
        return;
    }

    /* Configure CS pin as input with pull-up resistor */
    int ret = gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printk("Error configuring CS pin on the slave.\n");
        return;
    }

    printk("SPI Slave initialized...\n");

    while (1) {
        /* Wait until the master pulls CS low (active) */
        while (gpio_pin_get(gpio_dev, SPI_CS_PIN) == 1) {
            k_yield();  // Yield CPU until CS is active
        }

        printk("CS active, waiting for data...\n");

        /* SPI transaction */
        ret = spi_transceive(spi_dev, &spi_cfg, NULL, &rx);
        if (ret == 0) {
            printk("Received data: %02X\n", rx_buffer[0]);
        } else {
            printk("SPI communication error: %d\n", ret);
        }

        /* Wait until the master releases CS (inactive) */
        while (gpio_pin_get(gpio_dev, SPI_CS_PIN) == 0) {
            k_yield();
        }

        printk("CS inactive, waiting for next transaction...\n");
    }
}