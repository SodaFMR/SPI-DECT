#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#define SLEEP_TIME_MS   2000

#define GPIO_0_CS      20
#define GPIO0_NODE     DT_NODELABEL(gpio0)
#define SPI3_NODE      DT_NODELABEL(spi3)

static const struct device *spi3_dev = DEVICE_DT_GET(SPI3_NODE);
static const struct device *gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

static struct spi_cs_control cs_ctrl = {
    .gpio = SPI_CS_GPIOS_DT_SPEC_GET(SPI3_NODE), // Cambio aquí
    .delay = 0
};

static struct spi_config spi_cfg = {
    .operation = SPI_WORD_SET(8) | SPI_OP_MODE_MASTER, /* | SPI_MODE_CPOL | SPI_MODE_CPHA */
    .frequency = 1000000,   // 1 MHz
    .slave = 0,
    .cs = &cs_ctrl,  // Se pasa puntero
};

static void spi_init(void)
{
    int ret;

    if (!device_is_ready(spi3_dev)) {
        printk("SPI master device not ready!\n");
        return;
    }

    if (!device_is_ready(gpio0_dev)) {
        printk("GPIO0 device not ready!\n");
        return;
    }

    ret = gpio_pin_configure(gpio0_dev, GPIO_0_CS, GPIO_OUTPUT);
    if (ret < 0) {
        printk("Failed to configure pin %d\n", GPIO_0_CS);
    }

    if (spi_cs_is_gpio(spi3_dev)) {
        printk("SPI device has GPIO CS\n");
    } else {
        printk("SPI device has dedicated CS\n");
    }
}

static int spi_write_test_msg(void)
{
    int err;

    static uint8_t tx_buffer[1] = {0x5A}; // Valor inicial
    static uint8_t rx_buffer[1] = {0x00};

    const struct spi_buf tx_buf = {
        .buf = tx_buffer,
        .len = sizeof(tx_buffer)
    };
    const struct spi_buf_set tx = {
        .buffers = &tx_buf,
        .count = 1
    };

    struct spi_buf rx_buf = { // No puede ser const porque recibe datos
        .buf = rx_buffer,
        .len = sizeof(rx_buffer)
    };
    const struct spi_buf_set rx = {
        .buffers = &rx_buf,
        .count = 1
    };

    printk("CS is low\n");
    gpio_pin_set(gpio0_dev, GPIO_0_CS, 0);

    err = spi_transceive(spi3_dev, &spi_cfg, &tx, &rx);

    gpio_pin_set(gpio0_dev, GPIO_0_CS, 1);

    if (err) {
        printk("SPI error: %d\n", err);
    } else {
        printk("TX sent: %x\n", tx_buffer[0]);
        printk("RX received: %x\n", rx_buffer[0]);
        tx_buffer[0]++;
    }

    return err;
}

void main(void)
{
    printk("SPI Master example\n");

    spi_init();

    while (1) {
        spi_write_test_msg();
        k_msleep(SLEEP_TIME_MS);
    }
}
