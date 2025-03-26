#include <bcm2835.h>
#include <stdio.h>

// Definir los pines GPIO
#define SCLK_PIN RPI_V2_GPIO_P1_19  // GPIO 10 (SCLK)
#define MISO_PIN RPI_V2_GPIO_P1_11  // GPIO 17 (MISO)
#define CS_PIN   RPI_V2_GPIO_P1_15  // GPIO 22 (CS)

uint8_t counter = 0;

// Función para leer el estado de los pines
int check_pin_state(uint8_t pin) {
    // Leer el estado del pin
    uint8_t state = bcm2835_gpio_lev(pin);

    // Mostrar si el pin está en alto o bajo
    if (state == HIGH) {
    //    printf("El pin está en ALTO (HIGH)\n");
        return 1;
    } else {
    //    printf("El pin está en BAJO (LOW)\n");
        return 0;
    }
}



// Función para enviar un bit por MISO (Master In Slave Out)
void send_bit(uint8_t bit) {
    bcm2835_gpio_write(MISO_PIN, bit);
}

// Función para enviar un byte completo por MISO
void send_byte(uint8_t byte) {
    for (int i = 7; i >= 0; i--) {
        // Espera hasta que el reloj SPI (SCLK) tenga un flanco descendente
        uint8_t bit = (byte >> i) & 0x01;                                           send_bit(bit);
        while (check_pin_state(SCLK_PIN));                                          while (!check_pin_state(SCLK_PIN));
        //while (check_pin_state(SCLK_PIN));  // Espera a que el reloj baje>        //while (!check_pin_state(SCLK_PIN));   // Espera a que el reloj su>
    }
}


// MAIN FUNCTION
int main(int argc, char **argv) {
    // Inicializar la librería bcm2835
    if (!bcm2835_init()) {
        printf("Error al inicializar bcm2835!\n");
        return 1;
    }

    // Configurar los pines como entradas
    bcm2835_gpio_fsel(SCLK_PIN, BCM2835_GPIO_FSEL_INPT);  // SCLK como entr>    bcm2835_gpio_fsel(MISO_PIN, BCM2835_GPIO_FSEL_OUTP);  // MISO como entr>    bcm2835_gpio_fsel(CS_PIN, BCM2835_GPIO_FSEL_INPT);    // CS como entrada

    printf("Esperando comunicación SPI 1...\n");

    // Bucle para verificar continuamente el estado de los pines
    while (1) {
        if(check_pin_state(CS_PIN) != 1) {
            // printf("CS está en BAJO (LOW), se está realizando una comuni>            send_byte(counter);  // Enviar un byte de prueba
                counter = (counter + 1) % 256;
        };

        // Pausar por 1 segundo antes de verificar nuevamente
        //bcm2835_delay(1);  // Pausa de 1 segundo
        }

    // Limpiar los pines GPIO al salir
    bcm2835_close();
    return 0;
}
