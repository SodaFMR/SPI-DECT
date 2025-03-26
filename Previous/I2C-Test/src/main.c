#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

#define I2C_DEV     "I2C_2"  // Nombre del dispositivo I2C
#define I2C_ADDR    0xAA     // Dirección del esclavo I2C

void main(void) {
    const struct device *i2c_dev;
    uint8_t send_data = 0x55;  // Dato que se enviará al esclavo
    uint8_t recv_data;
    int ret;

    // Obtener el dispositivo I2C
    i2c_dev = device_get_binding(I2C_DEV);
    if (!i2c_dev) {
        printk("Error: No se encontró el dispositivo I2C.\n");
        return;
    }

    printk("Iniciando dispositivo I2C como maestro...\n");
    // Bucle principal para escribir y leer datos
    while (1) {

        // Intentar leer la respuesta del esclavo
        ret = i2c_read(i2c_dev, &recv_data, sizeof(recv_data), I2C_ADDR);
        if (ret == 0) {
            printk("Dato recibido correctamente del esclavo: 0x%02X\n", recv_data);
        } else {
            printk("Error al leer desde el bus I2C: %d\n", ret);
        }

        // Intentar leer la respuesta del esclavo
        ret = i2c_read(i2c_dev, &recv_data, sizeof(recv_data), I2C_ADDR);
        if (ret == 0) {
            printk("Dato recibido correctamente del esclavo: 0x%02X\n", recv_data);
        } else {
            printk("Error al leer desde el bus I2C: %d\n", ret);
        }

        // Esperar un segundo antes de realizar la siguiente transacción
        k_sleep(K_MSEC(1000));
    }
}