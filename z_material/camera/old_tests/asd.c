#include <zephyr/zephyr.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#define CS_GPIO_NAME "GPIO0" // Reemplaza esto con el nombre de tu dispositivo GPIO
#define CS_GPIO_PIN 20 // Reemplaza esto con el número de pin correcto

static const struct spi_config spi_cfg = {
    .operation = SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
    .frequency = 4000000,
    .slave = 0,
    .cs = NULL,
};

void main(void) {
    const struct device *spi_dev;
    const struct device *gpio_dev;
    int ret;

    gpio_dev = device_get_binding(CS_GPIO_NAME);
    if (gpio_dev == NULL) {
        printk("Could not get %s device\n", CS_GPIO_NAME);
        return;
    }

    ret = gpio_pin_configure(gpio_dev, CS_GPIO_PIN, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Failed to configure pin %d\n", CS_GPIO_PIN);
        return;
    }

    struct spi_cs_control cs_ctrl = {
        .gpio = gpio_dev,
        .gpio.pin = CS_GPIO_PIN,
        .delay = 0,
    };

    spi_dev = device_get_binding("SPI3");
    if (spi_dev == NULL) {
        printk("Could not get SPI device\n");
        return;
    }

    if (spi_cs_is_gpio(spi_dev)) {
        printk("SPI device has GPIO CS\n");
    } else {
        printk("SPI device has dedicated CS\n");
    }

    static uint8_t tx_buffer[1] = {0};
    const struct spi_buf tx_buf = {
        .buf = tx_buffer,
        .len = sizeof(tx_buffer),
    };
    const struct spi_buf_set tx = {
        .buffers = &tx_buf,
        .count = 1,
    };

    while (true) {
        // Activar el chip select del esclavo
        gpio_pin_set(gpio_dev, CS_GPIO_PIN, 0);

        // // Enviar los datos por SPI
        // ret = spi_write(spi_dev, &spi_cfg, &tx);
        // if (ret < 0) {
        //     printk("SPI write failed: %d\n", ret);
        // } else {
        //     printk("Enviado: %d\n", tx_buffer[0]);
        // }

        // Recibir los datos por SPI
        static uint8_t rx_buffer[1] = {0};
        const struct spi_buf rx_buf = {
            .buf = rx_buffer,
            .len = sizeof(rx_buffer),
        };
        const struct spi_buf_set rx = {
            .buffers = &rx_buf,
            .count = 1,
        };

        ret = spi_read(spi_dev, &spi_cfg, &rx);
        if (ret < 0) {
            printk("SPI read failed: %d\n", ret);
        } else {
            printk("Recibido: %d\n", rx_buffer[0]);
        }

        // Desactivar el chip select del esclavo
        gpio_pin_set(gpio_dev, CS_GPIO_PIN, 1);

        // Incrementar el valor del buffer de transmisión
        tx_buffer[0]++;

        // Esperar antes de enviar el siguiente valor
        k_msleep(3000); // Esperar 3 segundos antes de enviar el siguiente valor
    }
}
