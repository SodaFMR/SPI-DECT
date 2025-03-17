#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>

#define SLEEP_TIME_MS   1000

#define SPI3_NODE  DT_NODELABEL(spi3)
const struct device *spi3_dev = DEVICE_DT_GET(SPI3_NODE);

static struct k_poll_signal spi_slave_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_slave_done_sig);

static struct spi_config spi_slave_cfg = {
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_OP_MODE_SLAVE,
    .frequency = 4000000,
    .slave = 0,
};

static void spi_slave_init(void)
{
    if (!device_is_ready(spi3_dev)) {
        printk("ERROR: SPI slave device not ready!\n");
    }
}

static uint8_t slave_tx_buffer[2] = {0x55, 0xAA}; // Datos fijos para depuración
static uint8_t slave_rx_buffer[2];

static int spi_slave_wait_for_message(void)
{
    int signaled, result;
    k_poll_signal_check(&spi_slave_done_sig, &signaled, &result);
    return (signaled != 0) ? 0 : -1;
}

static int spi_slave_listen(void)
{
    const struct spi_buf s_tx_buf = {.buf = slave_tx_buffer, .len = sizeof(slave_tx_buffer)};
    const struct spi_buf_set s_tx = {.buffers = &s_tx_buf, .count = 1};

    struct spi_buf s_rx_buf = {.buf = slave_rx_buffer, .len = sizeof(slave_rx_buffer)};
    const struct spi_buf_set s_rx = {.buffers = &s_rx_buf, .count = 1};

    k_poll_signal_reset(&spi_slave_done_sig);

    printk("SPI Slave listening...\n");

    int error = spi_transceive_signal(spi3_dev, &spi_slave_cfg, &s_tx, &s_rx, &spi_slave_done_sig);
    if (error != 0) {
        printk("SPI Slave transceive error: %i\n", error);
        return error;
    }

    return 0;
}

int main(void)
{
    printk("SPI slave started\n");

    spi_slave_init();

    while (1) {
        if (spi_slave_wait_for_message() == 0) {
            // Imprimir los datos recibidos
            printk("SPI SLAVE RX: 0x%.2x, 0x%.2x\n", slave_rx_buffer[0], slave_rx_buffer[1]);

            // Mantener un buffer listo para la siguiente transacción
            static uint8_t counter = 0;
            slave_tx_buffer[0] = 0xA5;
            slave_tx_buffer[1] = counter++;
            printk("SPI SLAVE TX READY: 0x%.2x, 0x%.2x\n", slave_tx_buffer[0], slave_tx_buffer[1]);

            // Esperar la siguiente transacción
            spi_slave_listen();
        }
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
