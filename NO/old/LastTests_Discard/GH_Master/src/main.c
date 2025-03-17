#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

#define MY_SPI_MASTER DT_NODELABEL(spi3)
#define MY_SPI_MASTER_CS_DT_SPEC SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(reg_my_spi_master))

// SPI master functionality
const struct device *spi_dev;
static struct k_poll_signal spi_done_sig = K_POLL_SIGNAL_INITIALIZER(spi_done_sig);

static void spi_init(void)
{
	spi_dev = DEVICE_DT_GET(MY_SPI_MASTER);
	if(!device_is_ready(spi_dev)) {
		printk("SPI master device not ready!\n");
	}
	struct gpio_dt_spec spim_cs_gpio = MY_SPI_MASTER_CS_DT_SPEC;
	if(!device_is_ready(spim_cs_gpio.port)){
		printk("SPI master chip select device not ready!\n");
	}
}

static struct spi_config spi_cfg = {
	.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA,
	.frequency = 4000000,
	.slave = 0,
	.cs = {.gpio = MY_SPI_MASTER_CS_DT_SPEC, .delay = 0},
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

	struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = sizeof(rx_buffer),
	};
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	};

	// Update the TX buffer with a rolling counter
	tx_buffer[0] = counter++;
	printk("SPI TX: 0x%.2x, 0x%.2x\n", tx_buffer[0], tx_buffer[1]);

	// Reset signal
	k_poll_signal_reset(&spi_done_sig);
	
	// Start transaction
	int error = spi_transceive_signal(spi_dev, &spi_cfg, &tx, &rx, &spi_done_sig);
	if(error != 0){
		printk("SPI transceive error: %i\n", error);
		return error;
	}

	// Wait for the done signal to be raised and log the rx buffer
	int spi_signaled, spi_result;
	do{
		k_poll_signal_check(&spi_done_sig, &spi_signaled, &spi_result);
	} while(spi_signaled == 0);
	printk("SPI RX: 0x%.2x, 0x%.2x\n", rx_buffer[0], rx_buffer[1]);
	return 0;
}

int main(void)
{
	spi_init();

	spi_slave_init();

	printk("SPI master/slave example started\n");
	
	spi_slave_write_test_msg();

	while (1) {
		spi_write_test_msg();
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}