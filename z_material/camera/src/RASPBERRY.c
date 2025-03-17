#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(app);

/* Definiciones para el SPIM */
#define SPI_CS_PIN  20  // Pin GPIO para CS
#define SPI_DEV     "SPI_3"
#define GPIO_DEV    "GPIO_0"

/* Definir tiempo de delay en milisegundos (2000 ms = 2 segundos) */
#define DELAY_MS 1000

/* Configuración del buffer */
#define BUFFER_SIZE 1

static const struct spi_config spi_cfg = {
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .frequency = 200000, // Frecuencia SPI 1 MHz
    .slave = 0, // Dispositivo esclavo
    .cs = NULL, // La manejaremos manualmente
};

struct spi_cs_control spi_cs_ctrl;

void main(void)
{
    const struct device *spi_dev;
    const struct device *gpio_dev;
     uint8_t rx_buffer[BUFFER_SIZE] = {0}; // Buffer de recepción
    struct spi_buf rx_buf = {
        .buf = rx_buffer,
        .len = sizeof(rx_buffer),
    };
    struct spi_buf_set rx = {
        .buffers = &rx_buf,
        .count = 1,
    };

    /* Inicializar el dispositivo SPIM */
    spi_dev = device_get_binding(SPI_DEV);
    if (!spi_dev) {
        printk("Error: SPIM device not found.\n");
        return;
    }

    /* Obtener el dispositivo GPIO */
    gpio_dev = device_get_binding(GPIO_DEV);
    if (!gpio_dev) {
        printk("Error: GPIO device not found.\n");
        return;
    }

    /* Configurar el pin de CS como salida */
    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);

    while (1) {
        /* Activar la señal CS */
        // LOG_INF("CS is low\n");
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);

        /* Iniciar la transacción SPI */
        int ret = spi_transceive(spi_dev, &spi_cfg, NULL, &rx);
        if (ret == 0) {
            // LOG_INF("Data received: ");
            for (int i = 0; i < BUFFER_SIZE; i++) {
                LOG_INF("%02X ", rx_buffer[i]);
            }
            // LOG_INF("\n");
        } else {
            printk("SPIM transaction failed: %d\n", ret);
        }

        // k_msleep(500);

        /* Desactivar la señal CS */
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);
        // printk("CS is high again\n");
        /* Esperar 2 segundos antes de la próxima lectura */
        k_msleep(DELAY_MS);
    }
}