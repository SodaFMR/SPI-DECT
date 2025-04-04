#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>

#define SPI_DEV_NODE DT_NODELABEL(spi3)
#define SPI_CS_PIN   7  // Asegurar que coincide con la conexión física

// Configuración de buffers
#define BUFFER_SIZE 1024  // Debe coincidir con el esclavo
static uint8_t rx_buffer[BUFFER_SIZE] __aligned(32);  // Buffer alineado para DMA

static const struct device *spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
static const struct device *cs_gpio = DEVICE_DT_GET(DT_NODELABEL(gpio0));

// Configuración SPI optimizada
static const struct spi_config spi_cfg = {
    .frequency = 32000000,  // 32 MHz
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .slave = 0,
};

void print_optimized(const uint8_t *data, size_t len) {
    // Log reducido para máxima velocidad
    printk("Datos recibidos [%d bytes]:\n"
           "Cabecera: 0x%02X\n"
           "Muestra: 0x%02X 0x%02X 0x%02X 0x%02X...\n",
           len, data[0], data[1], data[2], data[3], data[4]);
}

void main(void) {
    // Configuración inicial
    if (!device_is_ready(spi_dev) {
        printk("Error: Dispositivo SPI no disponible\n");
        return;
    }

    // Configurar CS como GPIO
    gpio_pin_configure(cs_gpio, SPI_CS_PIN, GPIO_OUTPUT);
    gpio_pin_set(cs_gpio, SPI_CS_PIN, 1);  // CS inactivo

    // Configurar buffers SPI
    struct spi_buf rx_buf = {
        .buf = rx_buffer,
        .len = BUFFER_SIZE
    };
    const struct spi_buf_set rx_bufs = {
        .buffers = &rx_buf,
        .count = 1
    };

    printk("Maestro SPI inicializado. Esperando datos...\n");

    while (1) {
        // Iniciar transacción
        gpio_pin_set(cs_gpio, SPI_CS_PIN, 0);  // CS activo
        
        // Transacción SPI (solo recepción)
        int err = spi_transceive(spi_dev, &spi_cfg, NULL, &rx_bufs);
        
        gpio_pin_set(cs_gpio, SPI_CS_PIN, 1);  // CS inactivo

        if (err) {
            printk("Error SPI: %d\n", err);
            k_sleep(K_MSEC(100));
            continue;
        }

        // Verificar cabecera (primer byte)
        if ((rx_buffer[0] & 0xF0) != 0xF0) {  // Primeros 4 bits a 1
            printk("Error: Cabecera inválida 0x%02X\n", rx_buffer[0]);
            continue;
        }

        // Log de rendimiento optimizado
        print_optimized(rx_buffer, BUFFER_SIZE);
    }
}