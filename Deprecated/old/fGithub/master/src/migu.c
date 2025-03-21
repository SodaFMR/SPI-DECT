#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS   1000

#define GPIO_0_CS      20
#define GPIO0_NODE     DT_NODELABEL(gpio0)

#define SPI3_NODE DT_NODELABEL(spi3)

static const struct device *spi3_dev = DEVICE_DT_GET(SPI3_NODE);
static const struct device *gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

// static struct k_poll_signal spi_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_done_sig);

static void spi_init(void)
{
    if (!device_is_ready(spi3_dev)) {
        printk("SPI master device not ready!\n");
    }

    if (!device_is_ready(gpio0_dev)) {
        printk("GPIO0 device not ready!\n");
    }

    // Configurar CS como salida y ponerlo en alto por defecto (inactivo)
    gpio_pin_configure(gpio0_dev, GPIO_0_CS, GPIO_OUTPUT_ACTIVE);
    gpio_pin_set(gpio0_dev, GPIO_0_CS, 1); // CS inactivo al inicio
}

struct spi_cs_control cs_ctrl = {
    .gpio_dev = gpio0_dev,
    .gpio_pin = GPIO_0_CS,
    .delay 0,
};

static struct spi_config spi_cfg = {
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA,
    .frequency = 1000000,   // 1 MHz
    .slave = 0,
    .cs = cs_ctrl,
};

static int spi_write_test_msg(void)
{
    static uint8_t counter = 0;
    static uint8_t tx_buffer[2];
    static uint8_t rx_buffer[2];

    const struct spi_buf tx_buf = {
        .buf = tx_buffer,
        .len = sizeof(tx_buffer)
    };
    const struct spi_buf_set tx = {
        .buffers = &tx_buf,
        .count = 1
    };

    const struct spi_buf rx_buf = {
        .buf = rx_buffer,
        .len = sizeof(rx_buffer),
    };
    const struct spi_buf_set rx = {
        .buffers = &rx_buf,
        .count = 1
    };

    // Actualizar el buffer de transmisión con un contador
    tx_buffer[0] = counter++;
    printk("SPI TX: 0x%.2x, 0x%.2x\n", tx_buffer[0], tx_buffer[1]);

    // Resetear la señal de finalización
//    k_poll_signal_reset(&spi_done_sig);

    // Activar CS (poner en LOW)
    gpio_pin_set(gpio0_dev, GPIO_0_CS, 0);

    // Transmitir datos
    int error = spi_transceive_signal(spi3_dev, &spi_cfg, &tx, &rx, &spi_done_sig);
    
    // Desactivar CS (poner en HIGH)
    gpio_pin_set(gpio0_dev, GPIO_0_CS, 1);

    if (error != 0) {
        printk("SPI transceive error: %i\n", error);
        return error;
    }

    // Esperar la señal de finalización y leer el buffer de recepción
/*     
    int spi_signaled, spi_result;
    do {
        k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
    } while (spi_signaled == 0);
 */
    printk("SPI RX: 0x%.2x, 0x%.2x\n", rx_buffer[0], rx_buffer[1]);

    return 0;
}