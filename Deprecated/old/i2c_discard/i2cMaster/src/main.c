#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>

#define I2C_DEV DT_LABEL(DT_NODELABEL(i2c2))
#define SLAVE_ADDR 0x10  // Dirección del esclavo

void main(void) {
    const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c2));
    if (!device_is_ready(i2c_dev)) {
        printk("I2C no disponible\n");
        return;
    }

    uint8_t data = 0x5A;

    while (1) {
        int ret = i2c_write(i2c_dev, &data, sizeof(data), SLAVE_ADDR);
        if (ret < 0) {
            printk("Error en I2C Write: %d\n", ret);
        } else {
            printk("Dato enviado: 0x%X\n", data);
        }
        k_msleep(1000);
    }
}
