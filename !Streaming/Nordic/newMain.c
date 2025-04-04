#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#define SPI_DEV_NODE DT_NODELABEL(spi3)

// GPIO pin for the  DataReady and CS, if more slaves needed simply add more pins
#define SPI_CS_NODE DT_NODELABEL(gpio0)
#define SPI_CS_PIN  7

#define GPIO_DATAREADY 3

// Both GPIO and SPI devices are found here
static const struct device *spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
static const struct device *gpio_dev = DEVICE_DT_GET(SPI_CS_NODE);

static struct k_poll_signal spi_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_done_sig);

// Configuration for the SPI device
struct spi_config spi_cfg = {
    .frequency = 32000000,
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_HOLD_ON_CS,   // Mode 0, CPOL=0, CPHA=0, Keeps CS Low until the end of the transaction
    .slave = 0,
};

// Buffer size and buffer to store the received data
#define TOTAL_BYTES 65      // Maximum Bytes per iteration

static uint8_t recvbuf[TOTAL_BYTES] = {0};          // Buffer to store each iteration data
//static uint8_t sendbuf[TOTAL_BYTES] = {0};        // Buffer to store the data to send if needed

void main(void)
{
    // Configure the DataReady pin as input
    gpio_pin_configure(gpio_dev, GPIO_DATAREADY, GPIO_INPUT);

    // Configure the CS pin as output and ensuring it is innactive
    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

    // Configure the reception buffer
    struct spi_buf rx_buf = {
        .buf = recvbuf,
        .len = TOTAL_BYTES
    };
    struct spi_buf_set rx_buf_set = {
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



}