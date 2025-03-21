#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#define I2C_DEV DT_LABEL(DT_NODELABEL(i2c1))
#define SLAVE_ADDR 0x10  // Dirección del esclavo

uint8_t rx_buffer[1];

void main(void) {
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c2));
    if (!device_is_ready(i2c_dev)) {
        printk("I2C no disponible\n");
        return;
    }

    struct i2c_msg msgs[1];
    msgs[0].buf = rx_buffer;
    msgs[0].len = sizeof(rx_buffer);
    msgs[0].flags = I2C_MSG_READ | I2C_MSG_STOP;

    while (1) {
        int ret = i2c_transfer(i2c_dev, msgs, 1, SLAVE_ADDR);
        if (ret < 0) {
            printk("Error en I2C Read: %d\n", ret);
        } else {
            printk("Dato recibido: 0x%X\n", rx_buffer[0]);
        }
        k_msleep(1000);
    }
}
