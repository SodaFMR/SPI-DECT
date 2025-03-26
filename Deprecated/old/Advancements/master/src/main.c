#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#define CS_GPIO_NAME "GPIO_0"
#define CS_GPIO_PIN 7

#define SPI_DEV_NAME "SPI_3"

struct device *spi_dev;

struct spi_config spi_cfg;

const struct device *gpio_dev;

static void spi_init(void) 
{
    int ret;

    gpio_dev = device_get_binding(CS_GPIO_NAME); 
    if (gpio_dev == NULL) {
        printk("Could not get %s device\n", CS_GPIO_NAME);
        return;
    }

    ret = gpio_pin_configure(gpio_dev, CS_GPIO_PIN, GPIO_OUTPUT);
    if (ret < 0) {
        printk("Failed to configure pin %d\n", CS_GPIO_PIN);
        return;
    }

	struct spi_cs_control cs_ctrl = {
		.gpio = gpio_dev,
		.delay = 0,
	};


	spi_cfg.frequency = 1000000;
	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8);
	spi_cfg.slave = 0;
	spi_cfg.cs = cs_ctrl;

    spi_dev = device_get_binding(SPI_DEV_NAME);

	if (spi_dev == NULL) {
		printk("Could not get %s device\n", SPI_DEV_NAME);
		return;
	}

	if (!device_is_ready(spi_dev)) {
		printk("Device %s is not ready\n", spi_dev->name);
		return;
	}

	if(spi_cs_is_gpio(spi_dev)) {
		printk("SPI device has GPIO CS\n");
	} else {
		printk("SPI device has dedicated CS\n");
	}
}

void receive_test(void) 
{
    int err;

	static uint8_t receive_buffer[1];

    struct spi_buf tx_buf = {
        .buf = NULL,
        .len = 0
    };
    struct spi_buf rx_buf = {
        .buf = receive_buffer,
        .len = sizeof(receive_buffer)
    };

    struct spi_buf_set tx = {
        .buffers = &tx_buf,
        .count = 1
        };
    struct spi_buf_set rx = {
        .buffers = &rx_buf,
        .count = 1
    };

    printk("CS is low\n");
    gpio_pin_set(gpio_dev, CS_GPIO_PIN, 0);

    err = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);

    gpio_pin_set(gpio_dev, CS_GPIO_PIN, 1);
    printk("CS is high\n");

    if (err == 0) {
        printk("Received data: 0x%02X\n", receive_buffer[0]);
    } else {
        printk("SPI communication failed: %d\n", err);
    }
}

void main(void) {
    printk("SPI Master receive test\n");
    spi_init();
    
    while(1) {
        receive_test();
        k_msleep(1000);
    }
}
