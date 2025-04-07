#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#define SPI_DEV_NODE DT_NODELABEL(spi3)
#define SPI_CS_NODE DT_NODELABEL(gpio0)
#define SPI_CS_PIN  7
#define GPIO_DATAREADY 3

static const struct device *spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
static const struct device *gpio_dev = DEVICE_DT_GET(SPI_CS_NODE);
static struct gpio_callback data_ready_cb;
static volatile bool data_ready = false;

struct spi_config spi_cfg = {
    .frequency = 32000000,
<<<<<<< HEAD
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | 
                 SPI_TRANSFER_MSB | SPI_HOLD_ON_CS,
    .slave = 0,
};

#define TOTAL_BYTES 64
#define HEADER_SIZE 1
#define CRC_SIZE 2
#define PAYLOAD_SIZE (TOTAL_BYTES - HEADER_SIZE - CRC_SIZE)
#define MAX_DATA_SIZE 4000

static uint8_t recvbuf[TOTAL_BYTES];
static uint8_t data_buffer[MAX_DATA_SIZE];
static uint16_t total_bytes_received = 0;
static int16_t bytes_to_receive = MAX_DATA_SIZE;
static uint16_t corrupted_blocks = 0;

/* CRC-16 (Modbus) */
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

/* ISR para DataReady (flanco de subida) */
void data_ready_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    data_ready = true;
}

void init_gpio_interrupt(void) {
    // Config DataReady como entrada con interrupción
    gpio_pin_configure(gpio_dev, GPIO_DATAREADY, 
                      GPIO_INPUT | GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN);
    gpio_pin_interrupt_configure(gpio_dev, GPIO_DATAREADY, 
                                GPIO_INT_EDGE_RISING);
    
    gpio_init_callback(&data_ready_cb, data_ready_isr, BIT(GPIO_DATAREADY));
    gpio_add_callback(gpio_dev, &data_ready_cb);
}

bool process_received_block(void) {
    uint16_t received_crc = (recvbuf[TOTAL_BYTES - 2] << 8) | recvbuf[TOTAL_BYTES - 1];
    uint16_t calculated_crc = calculate_crc16(recvbuf, TOTAL_BYTES - CRC_SIZE);
    
    if (received_crc != calculated_crc) {
        corrupted_blocks++;
        return false;
    }

    uint16_t bytes_to_copy = PAYLOAD_SIZE;
    if (total_bytes_received + bytes_to_copy > MAX_DATA_SIZE) {
        bytes_to_copy = MAX_DATA_SIZE - total_bytes_received;
    }
    
    memcpy(&data_buffer[total_bytes_received], &recvbuf[HEADER_SIZE], bytes_to_copy);
    total_bytes_received += bytes_to_copy;
    bytes_to_receive -= bytes_to_copy;
    
    return true;
}

void print_received_data(void) {
    printk("\n=== Reception Complete ===\n");
    printk("Total bytes: %d\nCorrupted blocks: %d\n", 
           total_bytes_received, corrupted_blocks);
    printk("\nData (hex):\n");

    for (uint16_t i = 0; i < total_bytes_received; i += PAYLOAD_SIZE) {
        uint16_t bytes_in_block = (total_bytes_received - i) > PAYLOAD_SIZE ? 
                                 PAYLOAD_SIZE : (total_bytes_received - i);
        
        printk("\nBlock %d:\n", i/PAYLOAD_SIZE);
        for (uint16_t j = 0; j < bytes_in_block; j++) {
            printk("%02X ", data_buffer[i + j]);
            if ((j + 1) % 16 == 0) printk("\n");
        }
    }
}
=======
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_HOLD_ON_CS,   // Mode 0, CPOL=0, CPHA=0, Keeps CS Low until the end of the transaction
    .slave = 0,
};

// Buffer size and buffer to store the received data
#define TOTAL_BYTES 65      // Maximum Bytes per iteration
#define MAX_DATA_SIZE 4000   // Total size of the data to receive

static uint8_t recvbuf[TOTAL_BYTES] = {0};          // Buffer to store each iteration data
//static uint8_t sendbuf[TOTAL_BYTES] = {0};        // Buffer to store the data to send if needed
static uint8_t data_buffer[MAX_DATA_SIZE] = {0};    // Buffer to store the complete data
static uint16_t total_bytes_received = 0;           // Counter for the received Bytes
static int16_t bytes_to_receive = MAX_DATA_SIZE;    // Will be used to check the residual

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
<<<<<<< HEAD

    // Start of the SPI reception
    while (total_bytes_received < MAX_DATA_SIZE) {
        // Clean the buffer on each iteration
        memset(recvbuf, 0, TOTAL_BYTES);
>>>>>>> 486ca3ed37f137a260fb34ffaa5f4beddc757a65

void receive_spi_data(void) {
    struct spi_buf rx_buf = { .buf = recvbuf, .len = TOTAL_BYTES };
    struct spi_buf_set rx_buf_set = { .buffers = &rx_buf, .count = 1 };

    while (bytes_to_receive > 0) {
        if (data_ready) {
            data_ready = false;
            
            // Transacción SPI
            gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);
            int err = spi_transceive(spi_dev, &spi_cfg, NULL, &rx_buf_set);
            gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

            if (!err) {
                process_received_block();
            } else {
                printk("SPI error: %d\n", err);
            }
        }
<<<<<<< HEAD
        k_busy_wait(50); // Espera reducida
=======
        // 500 ms delay on each iteration
        k_msleep(500);
=======
    while (1) {

        while (gpio_pin_get(gpio_dev, GPIO_DATAREADY) == 0) {
            k_msleep(1);
        }
        // Start of the SPI reception
        while (bytes_to_receive > 0) {
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
                bytes_to_receive -= TOTAL_BYTES;

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
            // 50 ms delay on each iteration
            k_msleep(50);
        }
        if(gpio_pin_get(gpio_dev, GPIO_DATAREADY) == 0){
            printk("DataReady pin is low, waiting for the next data\n");
            break;
        } else {
            uint16_t remaining_bytes = MAX_DATA_SIZE % TOTAL_BYTES;
            if (remaining_bytes > 0) {
                printk("Remaining bytes: %d\n", remaining_bytes);
                memcpy(&data_buffer[total_bytes_received], &recvbuf[1], remaining_bytes);
                // Print the received block to check the correct reception
                //printk("Received data (block %d):\n", seq_num);
                for (int i = 0; i < remaining_bytes; i++) {
                    printk("%02X ", recvbuf[i + 1]);
                    if ((i + 1) % 16 == 0) {
                        printk("\n");
                    }
                }
                printk("\n- - - - - - - - - - - - - - - - - - - - - - - -\n");
            }
        }
        break;
>>>>>>> limits
>>>>>>> 486ca3ed37f137a260fb34ffaa5f4beddc757a65
    }
    print_received_data();
}

int main(void) {
    // 1. Inicialización GPIO
    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);
    gpio_pin_set(gpio_dev, SPI_CS_PIN, 1); // CS inactivo
    
    // 2. Verificar dispositivos
    if (!device_is_ready(spi_dev)) {
        printk("SPI device not ready\n");
        return -1;
    }
    
    // 3. Configurar interrupción
    init_gpio_interrupt();
    printk("SPI Master ready\n");
    
    // 4. Bucle principal
    receive_spi_data();
    
    while (1) {
        k_sleep(K_SECONDS(1));
    }
    return 0;
}