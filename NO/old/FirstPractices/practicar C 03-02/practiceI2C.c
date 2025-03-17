#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>  // Para usar sleep()

// Definición de una estructura que simula un dispositivo esclavo
struct SimulatedDevice {
    uint8_t buffer[256]; // Memoria simulada
    uint8_t data_ready;  // Indica si hay un dato disponible
};

// Inicializamos un "dispositivo esclavo" en memoria
struct SimulatedDevice slave_device;

// Función que simula escribir en el esclavo (como i2c_write)
void write_to_device(struct SimulatedDevice *dev, uint8_t data) {
    dev->buffer[0] = data;   // Guardar el dato en la primera posición
    dev->data_ready = 1;     // Indicar que hay un dato listo
    printf("[MAESTRO] Dato enviado al esclavo: 0x%02X\n", data);
}

// Función que simula leer del esclavo (como i2c_read)
uint8_t read_from_device(struct SimulatedDevice *dev) {
    if (dev->data_ready) {
        dev->data_ready = 0;  // Marcar que el dato ha sido leído
        printf("[ESCLAVO] Dato recibido correctamente: 0x%02X\n", dev->buffer[0]);
        return dev->buffer[0];
    } else {
        printf("[ESCLAVO] No hay datos nuevos disponibles.\n");
        return 0xFF;  // Valor de error
    }
}

int main() {
    uint8_t send_data = 0x55;  // Dato a enviar
    uint8_t recv_data;

    printf("Simulación de comunicación maestro-esclavo iniciada...\n");

    while (1) {
        // Simular escritura al esclavo
        write_to_device(&slave_device, send_data);

        // Simular lectura del esclavo
        recv_data = read_from_device(&slave_device);

        // Esperar 1 segundo (similar a k_sleep en Zephyr)
        sleep(1);
    }

    return 0;
}
