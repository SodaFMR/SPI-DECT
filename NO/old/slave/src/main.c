#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#define SPI3_NODE        DT_NODELABEL(spi3)
static const struct device *spi3_dev = DEVICE_DT_GET(SPI3_NODE);

#define GPIO_CS_PIN      7                    // Mismo pin que en el maestro
#define GPIO_CS_NODE     DT_NODELABEL(gpio0)  // GPIO0 donde llega la señal CS
static const struct device *gpio0_dev = DEVICE_DT_GET(GPIO_CS_NODE);

static struct spi_config spi_cfg = {
        .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
        .frequency = 8000000U,  
        .slave = 1,  // Se configura como esclavo
};

void main(void)
{
    if (!device_is_ready(spi3_dev)) {
        printk("SPI3 device not ready\n");
        return;
    }

    if (!device_is_ready(gpio0_dev)) {
        printk("GPIO device not ready\n");
        return;
    }

    // Configurar GPIO_CS_PIN como entrada para detectar el CS del maestro
    gpio_pin_configure(gpio0_dev, GPIO_CS_PIN, GPIO_INPUT);

    uint8_t tx_data = 0xAD;  // Dato a enviar al maestro

    struct spi_buf tx_spi_buf = {
            .buf = &tx_data,
            .len = sizeof(tx_data),
    };

    struct spi_buf_set spi_tx_buffer_set = {
            .buffers = &tx_spi_buf,
            .count = 1,
    };

    while (1) {
        // Esperar a que el maestro active CS (LOW)
        while (gpio_pin_get(gpio0_dev, GPIO_CS_PIN) != 0) {
            k_msleep(1);  // Cede el procesador mientras espera
        }

        // CS está en LOW, transmitir dato
        int err = spi_write(spi3_dev, &spi_cfg, &spi_tx_buffer_set);
    
        if (err < 0) {
            printk("SPI slave write failed: %d\n", err);
        } else {
            printk("Dato enviado al maestro: 0x%X\n", tx_data);
        }
        while(gpio_pin_get(gpio0_dev, GPIO_CS_PIN) == 0) {
            k_msleep(1);  // Cede el procesador mientras espera
        }
    }
}
