#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

/* 1000 ms = 1 sec */
#define SLEEP_TIME_MS 1000

#define SPI3_NODE DT_NODELABEL(spi3)
static const struct device *spi3_dev = DEVICE_DT_GET(SPI3_NODE);

#define GPIO_0_CS      20
#define GPIO0_NODE     DT_NODELABEL(gpio0) 
const struct device *gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

static struct spi_config spi_cfg = {
        .frequency = 125000U,
        .operation = SPI_WORD_SET(8),
        .slave = 0,
};

static uint8_t receiveSlave(void) {
    int err;
    uint8_t rx_data[1] = {0x00};   // Buffer de recepción

    struct spi_buf rx_spi_buf = {
            .buf = rx_data,
            .len = sizeof(rx_data),
    };

    struct spi_buf_set spi_rx_buffer_set = {
            .buffers = &rx_spi_buf,
            .count = 1,
    };

    gpio_pin_set(gpio0_dev, GPIO_0_CS, 0);  // Activa CS (LOW)
    err = spi_read(spi3_dev, &spi_cfg, &spi_rx_buffer_set);
    gpio_pin_set(gpio0_dev, GPIO_0_CS, 1);  // Desactiva CS (HIGH)

    if (err < 0) {
            printk("SPI slave receive failed: %d\n", err);
            return 0xFF;  // Retorna un valor de error
    }

    return rx_data[0];  // Devuelve el dato recibido
}

void main(void)
{       if (!device_is_ready(spi3_dev)) {
        printk("SPI3 device not ready\n");
        return;
}    
    gpio_pin_configure(gpio0_dev, GPIO_0_CS, GPIO_OUTPUT);
    gpio_pin_set(gpio0_dev, GPIO_0_CS, 1);

    while (1) {
        uint8_t data = receiveSlave();
        printk("Dato recibido del esclavo: 0x%X\n", data);
        k_msleep(SLEEP_TIME_MS);
        gpio_pin_set(gpio0_dev, GPIO_0_CS, 0);
        k_msleep(SLEEP_TIME_MS*5);
        gpio_pin_set(gpio0_dev, GPIO_0_CS, 1);
    }
}