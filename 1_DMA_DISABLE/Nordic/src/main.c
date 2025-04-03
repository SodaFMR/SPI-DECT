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
        k_busy_wait(50); // Espera reducida
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