#include <bcm2835.h>
#include <stdio.h>

// Definir los pines GPIO
#define SCLK_PIN RPI_V2_GPIO_P1_19  // GPIO 10 (SCLK)
#define MISO_PIN RPI_V2_GPIO_P1_11  // GPIO 17 (MISO)
#define CS_PIN   RPI_V2_GPIO_P1_15  // GPIO 22 (CS)

uint8_t data[693];
int j = 0;

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

int wait_clk(){
    while (check_pin_state(SCLK_PIN));
    while (!check_pin_state(SCLK_PIN));
    return 0;
}


int receive_byte(){
    uint8_t byte = 0;
    for (int i = 7; i >= 0; i--) {
        byte = byte | (check_pin_state(MISO_PIN) << i);
        data[j] = byte;
        j++;
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
    bcm2835_gpio_fsel(SCLK_PIN, BCM2835_GPIO_FSEL_INPT);  // SCLK como entrada
    bcm2835_gpio_fsel(MISO_PIN, BCM2835_GPIO_FSEL_INPT);  // MISO como entrada
    bcm2835_gpio_fsel(CS_PIN, BCM2835_GPIO_FSEL_INPT);    // CS como entrada

    printf("Esperando recepcion de datos...\n");

    // Bucle para verificar continuamente el estado de los pines
    while (1) {
        if(check_pin_state(CS_PIN) != 1) {
            // printf("CS está en BAJO (LOW), se está realizando una comunicación SPI\n");
            wait_clk();  // Enviar un byte de prueba
            receive_byte();
        };
        if(j >= 693){
            printf("Datos recibidos: ");
            for(int i = 0; i < 693; i++){
                printf("%d", data[i]);
            }
            printf("\n");
            j = 0;
        }
        // Pausar por 1 segundo antes de verificar nuevamente
        //bcm2835_delay(1);  // Pausa de 1 segundo
    }

    // Limpiar los pines GPIO al salir
    bcm2835_close();
    return 0;
}