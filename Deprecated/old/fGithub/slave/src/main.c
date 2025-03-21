#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS   2000

#define GPIO_0_CS      20
#define GPIO0_NODE     DT_NODELABEL(gpio0)
#define SPI3_NODE      DT_NODELABEL(spi3)

static const struct device *spi3_dev = DEVICE_DT_GET(SPI3_NODE);
static const struct device *gpio0_dev = DEVICE_DT_GET(GPIO0_NODE);

static struct spi_config spi_slave_cfg = {
    .operation = SPI_OP_MODE_SLAVE | SPI_WORD_SET(8),
    .frequency = 1000000,
    .slave = 0,
};

static void spi_slave_init(void)
{
    int ret;

    ret = gpio_pin_configure(gpio0_dev, GPIO_0_CS, GPIO_INPUT);
    if (ret < 0) {
        printk("Failed to configure pin %d\n", GPIO_0_CS);
        return;
    }

    if (!device_is_ready(spi3_dev)) {
        printk("ERROR: SPI slave device not ready!\n");
    }

    if (spi_cs_is_gpio(spi3_dev)) {
        printk("SPI device has GPIO CS\n");
    } else {
        printk("SPI device has dedicated CS\n");
    }

    if (!device_is_ready(gpio0_dev)) {
        printk("GPIO0 device not ready!\n");
    }
}

static int spi_slave_listen(void)
{
    int err;

    static uint8_t slave_tx_buffer[1] = {0x5A}; // Datos fijos para depuración
    static uint8_t slave_rx_buffer[1];

    const struct spi_buf s_tx_buf = {
        .buf = slave_tx_buffer, 
        .len = sizeof(slave_tx_buffer)
    };
    
    const struct spi_buf_set s_tx = {
        .buffers = &s_tx_buf, 
        .count = 1
    };

    struct spi_buf s_rx_buf = {
        .buf = slave_rx_buffer, 
        .len = sizeof(slave_rx_buffer)
    };
    
    const struct spi_buf_set s_rx = {
        .buffers = &s_rx_buf, 
        .count = 1
    };

    while (gpio_pin_get(gpio0_dev, GPIO_0_CS) == 1) {
        k_msleep(1);
    }
    printk("CS is low\n");

    err = spi_transceive(spi3_dev, &spi_slave_cfg, &s_tx, &s_rx);

    if (err) {
        printk("SPI error: %d\n", err);
    } else {
        printk("TX sent: %x\n", slave_tx_buffer[0]);
        printk("RX received: %x\n", slave_rx_buffer[0]);
        slave_rx_buffer[0]++;
    }

    return err;
}

int main(void)
{
    printk("SPI slave started\n");
    spi_slave_init();

    while (1) {
        spi_slave_listen();
        k_msleep(SLEEP_TIME_MS);
    }
}
