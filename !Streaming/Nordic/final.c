#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>

#define SPI_DEV_NODE DT_NODELABEL(spi3)
#define TOTAL_BYTES 1024  // Debe coincidir con el esclavo

static const struct device *spi_dev = DEVICE_DT_GET(SPI_DEV_NODE);
static struct k_poll_signal spi_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_done_sig);

// Buffer alineado para DMA (32 bytes para optimizar caché)
static uint8_t rx_buffer[TOTAL_BYTES] __aligned(32);

static const struct spi_config spi_cfg = {
    .frequency = 32000000,  // 32 MHz
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .slave = 0,
    .cs = {
        .gpio = SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(your_cs_node)), // Ajustar en devicetree
        .delay = 0
    }
};

void print_optimized(const uint8_t *data) {
    // Imprime solo los primeros y últimos 4 bytes para máxima velocidad
    printk("Recibido: [%02X %02X %02X %02X...%02X %02X %02X %02X]\n",
           data[0], data[1], data[2], data[3],
           data[TOTAL_BYTES-4], data[TOTAL_BYTES-3],
           data[TOTAL_BYTES-2], data[TOTAL_BYTES-1]);
}

void main(void) {
    if (!device_is_ready(spi_dev)) {
        printk("Error: Dispositivo SPI no disponible\n");
        return;
    }

    const struct spi_buf rx_buf = {
        .buf = rx_buffer,
        .len = TOTAL_BYTES
    };
    const struct spi_buf_set rx_bufs = {
        .buffers = &rx_buf,
        .count = 1
    };

    printk("Maestro SPI iniciado. Recibiendo datos...\n");

    while (1) {
        k_poll_signal_reset(&spi_done_sig);
        
        // Transacción asíncrona
        int err = spi_transceive_signal(spi_dev, &spi_cfg, NULL, &rx_bufs, &spi_done_sig);
        if (err) {
            printk("Error SPI: %d\n", err);
            k_sleep(K_MSEC(10));
            continue;
        }

        // Esperar señal de completado
        int signaled, result;
        do {
            k_poll_signal_check(&spi_done_sig, &signaled, &result);
        } while (!signaled);

        // Mostrar datos optimizado
        print_optimized(rx_buffer);
    }
}