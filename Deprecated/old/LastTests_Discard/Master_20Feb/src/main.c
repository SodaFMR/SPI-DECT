#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// #define SPI_SLAVE_NODE_0 DT_NODELABEL(spi3)
// #define CS_GPIO_NAME DT_NODELABEL(gpio0)

// #include <zephyr.h>
// #include <misc/printk.h>

// Definitions for GPIO
#define CS_GPIO_NAME "GPIO_0" // Reemplaza esto con el nombre de tu dispositivo GPIO
#define CS_GPIO_PIN 7 // Reemplaza esto con el número de pin correcto

// Definitions for SPI3
#define SPI_NODE_NAME "SPI_3"


// static const struct spi_config spi_cfg = {
// 	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
// 		     SPI_MODE_CPOL | SPI_OP_MODE_MASTER | SPI_MODE_CPHA,
// 	.frequency = 1000000
// };

struct device *spi_dev;		// Representa el dispositivo SPI
	// Valor almacenado en la direccion spi_dev (por lo de *spi_dev) es un puntero a una estructura que representa el dispositivo SPI
struct spi_config spi_cfg;	// Almacena la configuracion del dispositivo ISP

const struct device *gpio_dev;	// Representa el dispositivo GPIO que controla CS (chip select)
	// Valor almacenado en la direccion gpio_dev (por lo de *gpio_dev) es un puntero a una estructura que representa el dispositivo GPIO


static void spi_init(void)
{
	
    int ret;	// Entero creado para almacenar la salida del pin CS_GPIO_PIN

    gpio_dev = device_get_binding(CS_GPIO_NAME); // Obtiene el dispositivo GPIO con el nombre CS_GPIO_NAME y lo almacena dentro de gpio_dev
    if (gpio_dev == NULL) {
        printk("Could not get %s device\n", CS_GPIO_NAME);	// %s indica la cadena de caracteres que es el nombre del dispositivo GPIO
        return;
    }

    ret = gpio_pin_configure(gpio_dev, CS_GPIO_PIN, GPIO_OUTPUT);	// Configura el pin CS_GPIO_PIN como salida en el dispositivo GPIO y lo almacena en ret
    if (ret < 0) {
        printk("Failed to configure pin %d\n", CS_GPIO_PIN);	// %d indica el entero que es el numero de pin
        return;
    }

	struct spi_cs_control cs_ctrl = {
		.gpio = gpio_dev,
		.delay = 0
	};


	spi_cfg.frequency = 8000000;
	spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8);
	spi_cfg.slave = 0;
	spi_cfg.cs = cs_ctrl;

	// spi_dev = DEVICE_DT_GET(SPI_SLAVE_NODE_0);
    spi_dev = device_get_binding(SPI_NODE_NAME);

	// const char* const spiName = "SPI3";
	// spi_dev = device_get_binding(spiName);

	if (spi_dev == NULL) {
		printk("Could not get %s device\n", SPI_NODE_NAME);
		return;
	}

	// Check if device is ready
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

void spi_test_send(void)
{
	int err;
	static uint8_t tx_buffer[1];
	static uint8_t rx_buffer[1];

	const struct spi_buf tx_buf = {
		.buf = tx_buffer,
		.len = sizeof(tx_buffer)
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

	struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = sizeof(rx_buffer),
	};
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	};

    // while(gpio_pin_get(gpio_dev, CS_GPIO_PIN) == 1) {
    //     k_msleep(1);
    // }
    printk("CS is low\n");
	gpio_pin_set(gpio_dev, CS_GPIO_PIN, 0);
	// err = spi_transceive(spi_dev, &spi_cfg, NULL, &rx);


	// Transfer function (Asynchronous)
	err = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);

	// err = spi_read(spi_dev, &spi_cfg, &rx);
    // k_msleep(5000);

	gpio_pin_set(gpio_dev, CS_GPIO_PIN, 1);

	// err = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);


	if (err) {
		printk("SPI error: %d\n", err);
	} else {
		/* Connect MISO to MOSI for loopback */
		printk("TX sent: %x\n", tx_buffer[0]);
		printk("RX recv: %x\n", rx_buffer[0]);
		tx_buffer[0]++;
	}	
}

void main(void)
{
	printk("SPIM Example\n");
	spi_init();

	while (1) {
		spi_test_send();
		k_msleep(2000);
	}

}