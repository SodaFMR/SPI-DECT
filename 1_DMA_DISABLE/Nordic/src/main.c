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

// Configuration for the SPI device
struct spi_config spi_cfg = {
    .frequency = 7500000,
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_HOLD_ON_CS,   // Mode 0, CPOL=0, CPHA=0, Keeps CS Low until the end of the transaction
    .slave = 0,
};

// Buffer size and buffer to store the received data
#define TOTAL_BYTES 65      // Maximum Bytes per iteration
#define MAX_DATA_SIZE 1000   // Total size of the data to receive

static uint8_t recvbuf[TOTAL_BYTES] = {0};          // Buffer to store each iteration data
//static uint8_t sendbuf[TOTAL_BYTES] = {0};        // Buffer to store the data to send if needed
static uint8_t data_buffer[MAX_DATA_SIZE] = {0};    // Buffer to store the complete data
static uint16_t total_bytes_received = 0;           // Counter for the received Bytes

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
    printk("Initializing SPI in nRF9161 as Master...\n");

    // Check the devices to be ready
    if (!device_is_ready(spi_dev)) {
        printk("Error: SPI3 is not ready.\n");
        return;
    }
    if (!device_is_ready(gpio_dev)) {
        printk("Error: GPIO is not ready.\n");
        return;
    }

    // Start of the SPI reception
    while (total_bytes_received < MAX_DATA_SIZE) {
        if(gpio_pin_get(gpio_dev, GPIO_DATAREADY) == 0){
            printk("DataReady pin is low, waiting for the signal to be high\n");
            while(gpio_pin_get(gpio_dev, GPIO_DATAREADY) == 0){
                k_msleep(1);
            }
        }
        printk("DataReady pin is high, starting the SPI communication\n");
        
        // Clean the buffer on each iteration
        memset(recvbuf, 0, TOTAL_BYTES);

        // CS is set to low to start receiving
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);

        // Receive the data from the SPI slave
        int err = spi_transceive(spi_dev, &spi_cfg, NULL, &rx_buf_set);

        // CS is set to high on the end of the communication
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

        if (err) {
            printk("Error in spi_read(), failed to receive: %d\n", err);
        } else {
            // Check the header to obtain the sequence number from the 7 Less Singificant Bits
            uint8_t seq_num = recvbuf[0] & 0x7F;
            printk("Received block with sequence number: %d\n", seq_num);

            // Store the data received in the total buffer
            uint16_t bytes_to_copy = TOTAL_BYTES - 1;  // Here the first byte is excluded (header)
            memcpy(&data_buffer[total_bytes_received], &recvbuf[1], bytes_to_copy);
            total_bytes_received += bytes_to_copy;

            // Print the received block to check the correct reception
            printk("Received data (block %d):\n", seq_num);
            for (int i = 0; i < bytes_to_copy; i++) {
                printk("%02X ", recvbuf[i + 1]);
                if ((i + 1) % 16 == 0) {
                    printk("\n");
                }
            }
            printk("- - - - - - - - - - - - - - - - - - - - - - - -\n");

            // Check if we hace received all the data (MAX_DATA_SIZE bytes)
            if (total_bytes_received >= MAX_DATA_SIZE) {
                printk("All data received: %d bytes\n", total_bytes_received);
                
                // Here we can store or use the collected data so far

            }
        }
        // 500 ms delay on each iteration
        k_msleep(50);
    }
}