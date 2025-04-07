#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <string.h>


#define SPI_DEV_NODE DT_NODELABEL(spi3)
#define BUFFER_SIZE 1024

static const struct device *spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
static uint8_t recbuff[BUFFER_SIZE] __aligned(32);

static struct k_poll_signal spi_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_done_sig);

static const struct spi_config spi_cfg = {
    .frequency = 32000000,
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .slave = 0,
    .cs = SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(spi3)),
};

int main(void) {
    
    if (!device_is_ready(spi_dev)) {
        printk("Error: SPI device not ready\n");
        return -1;
    }

    const struct spi_buf rx_buf = {
        .buf = recbuff,
        .len = BUFFER_SIZE
    };
    const struct spi_buf_set rx_bufs = {
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

    while (1) {
        k_poll_signal_reset(&spi_done_sig);
        int err = spi_transceive_signal(spi_dev, &spi_cfg, NULL, &rx_bufs, &spi_done_sig);
        
        if (err) {
            printk("Error: %d\n", err);
            k_sleep(K_MSEC(10));
            continue;
        }

        // Versión similar al código referencia (polling activo)
        int signaled;
        do {
            k_poll_signal_check(&spi_done_sig, &signaled, NULL);
        } while (!signaled);

        printk("Datos recibidos. Primer byte: 0x%02X\n", recbuff[0]);
    }
}