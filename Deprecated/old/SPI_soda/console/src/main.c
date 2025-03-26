#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/console/console.h>
#include <dk_buttons_and_leds.h>

#define SPI_DEV    "SPI_3"
#define SPI_CS_PIN 20
#define GPIO_DEV   "GPIO_0"

#define BUFFER_SIZE 1
#define DELAY 1000

static const struct spi_config spi_cfg = {
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_TRANSFER_MSB,
    .frequency = 8000000,
    .slave = 0,
    .cs = NULL,
};

bool transmitting = false;  // Transmission state

// Button handler for controlling transmission
void button_handler(uint32_t button_state, uint32_t has_changed)
{
    if (has_changed & button_state & DK_BTN1_MSK) {
        transmitting = true;
        printk("Starting SPI transmission...\n");
    }
    if (has_changed & button_state & DK_BTN2_MSK) {
        transmitting = false;
        printk("Stopping SPI transmission...\n");
    }
    if (has_changed & button_state & DK_BTN3_MSK) {
        printk("Resetting system...\n");
        k_sleep(K_SECONDS(1));
        sys_reboot(SYS_REBOOT_COLD);
    }
    if (has_changed & button_state & DK_BTN4_MSK) {
        printk("Button 4: Nothing Happened\n");
    }
}

void main(void)
{
    const struct device *spi_dev;
    const struct device *gpio_dev;
    
    uint8_t tx_buffer[BUFFER_SIZE] = {0xAD};  
    struct spi_buf tx_buf = {
        .buf = tx_buffer,
        .len = sizeof(tx_buffer),
    };
    struct spi_buf_set tx = {
        .buffers = &tx_buf,
        .count = 1,
    };

    // Initialize devices
    spi_dev = device_get_binding(SPI_DEV);
    if (!spi_dev) {
        printk("Error: No SPI device found.\n");
        return;
    }

    gpio_dev = device_get_binding(GPIO_DEV);
    if (!gpio_dev) {
        printk("Error: No GPIO device found.\n");
        return;
    }

    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);

    // Initialize buttons and LEDs
    dk_buttons_init(button_handler);
    dk_leds_init();

    printk("Press Button 1 to start, Button 2 to stop, Button 3 to reset, Button 4 does nothing.\n");

    while (1) {
        // Check the transmission state
        if (transmitting) {
            gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);
            int ret = spi_transceive(spi_dev, &spi_cfg, &tx, NULL);
            gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

            if (ret == 0) {
                printk("Sent: %02X\n", tx_buffer[0]);
                tx_buffer[0]++;
            } else {
                printk("SPI transmission failed: %d\n", ret);
            }

            k_msleep(DELAY);
        }
    }
}
