#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/printk.h>
#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

// Codigo de probar la comunicación SPI con una raspberry Pi

LOG_MODULE_REGISTER(app);

/* Definiciones para el SPIM */
#define SPI_CS_PIN  20  // Pin GPIO para CS (Chip Select)
#define SPI_DEV     "SPI_3" // Nombre del dispositivo SPI
#define GPIO_DEV    "GPIO_0" // Nombre del dispositivo GPIO

/* Definir tiempo de delay en milisegundos (2000 ms = 2 segundos) */
#define DELAY_MS 2000

/* Configuración del buffer */
#define BUFFER_SIZE 1   // Indica que solo tiene un byte de informacion

/* Parametros para la configuracion de SPI, siempre se tienen que hacer */
// STATIC indica que es una variable o funcion que solo se puede usar en este archivo
// CONST indica que no se puede modificar
// struct spi_config es la estructura que se usa para configurar el periférico SPI. Esta estructura es proporcionada por la API de Zephyr para configurar un bus SPI.
// spi_cfg es el nombre de la variable que contiene la configuración de SPI. Esta variable se pasará a la función SPI para inicializar el bus SPI con estas configuraciones.
static const struct spi_config spi_cfg = {  // Cuando pone .algo significa que es un apartado de la estructura que hay que configurar
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,  // SPI_WORD_SET(8) indica que se van a transmitir datos de 8 bits (1 byte)
                                                      // Se añade | para concatenar ya que otra configuracio que hay que realiar a operation es SPI_TRANSFER_MSB que
                                                      // indica que se va a transmitir el bit más significativo primero.
    .frequency = 200000, // Frecuencia SPI 200kHz, indica la velocidad a la que se van a tranmsitir los datos
    .slave = 0, // Este campo indica si estamos trabajando con un dispositivo esclavo o maestro; 0 es Maestro, y 1 es Esclavo
    .cs = NULL, // Lo ponemos a NULL porque queremos manejar manualmente el CHIP SELECT nosotros en base a otras funciones
};

struct spi_cs_control spi_cs_ctrl;  // SPI_CS_CONTROL es la struct que nos permite realizar el manejo manual del CHIP SELECT
                                    // dicho de otra forma, es un controlador que nos permite activar y desactivar el CS manualmente

// En el programa principal hay que hacer varias configuraciones
void main(void)
{
    const struct device *spi_dev;   // Declara un puntero a una estructura device que representará el dispositivo SPI. Es un puntero porque el dispositivo
                                    //  SPI se obtendrá dinámicamente durante la ejecución.
    const struct device *gpio_dev;  // Declara un puntero a una estructura device que representará el dispositivo GPIO. El GPIO será utilizado para manejar
                                    // el pin de CS (Chip Select) del dispositivo SPI.
    uint8_t tx_buffer[BUFFER_SIZE] = {0xAA};    // eclara un buffer de transmisión (tx_buffer) con un tamaño de BUFFER_SIZE. En este caso, el buffer contiene
                                                // el valor 0xAA (un byte). Este buffer se enviará a través del bus SPI.
    struct spi_buf tx_buf = {       // Declara una estructura spi_buf que es utilizada por Zephyr para configurar los buffers de datos para las
                                    // transacciones SPI. En este caso, la estructura tx_buf se utiliza para configurar el buffer de transmisión.
        .buf = tx_buffer,           // El puntero al buffer de datos de transmisión es tx_buffer
        .len = sizeof(tx_buffer),   // El tamaño del buffer de transmisión es el tamaño de tx_buffer
    };
    struct spi_buf_set tx = {       // Declara una estructura spi_buf_set que agrupa los buffers para la transacción SPI. En este caso, estamos
                                    //  utilizando un solo buffer, tx_buf
        .buffers = &tx_buf,         // El puntero al conjunto de buffers es tx_buf
        .count = 1,                 // El número de buffers que estamos utilizando para la transacción es 1, ya que solo un buffer es el que transmitimos
    };

    /* Inicializar el dispositivo SPIM */
    /*
    Llama a la función device_get_binding() para obtener el dispositivo SPI. SPI_DEV es el nombre
    del dispositivo SPI que se configuró en el proyecto (por ejemplo, "SPI_1"). El puntero spi_dev
    se vincula al dispositivo SPI correspondiente.
    */ 
    spi_dev = device_get_binding(SPI_DEV);
    if (!spi_dev) {
        printk("Error: SPIM device not found.\n");
        return;
        // Verifica si el dispositivo SPI se ha vinculado correctamente. Si no, imprime un mensaje de error y sale de la función main()
    }

    /* Obtener el dispositivo GPIO */
    /*
    Llama a la función device_get_binding() para obtener el dispositivo GPIO. GPIO_DEV es el
    nombre del dispositivo GPIO que se configuró previamente (por ejemplo, "GPIO_0"). El puntero
    gpio_dev se vincula al dispositivo GPIO correspondiente.    
    */
    gpio_dev = device_get_binding(GPIO_DEV);
    if (!gpio_dev) {
        printk("Error: GPIO device not found.\n");
        return;
        // Verifica si el dispositivo GPIO se ha vinculado correctamente. Si no, imprime un mensaje de error y sale de la función main()
    }

    /* Configurar el pin de CS como salida */
    /*
    Configura el pin de Chip Select (CS) como una salida en el dispositivo GPIO. SPI_CS_PIN 
    es el número del pin que se utilizará para la señal de CS
    */
    gpio_pin_configure(gpio_dev, SPI_CS_PIN, GPIO_OUTPUT);

    while (1) {
        /* Activa la señal CS. La función gpio_pin_set() cambia el valor del pin de CS a 0 (activo bajo).
           Esto indica al dispositivo esclavo que se inicia una transacción SPI */
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 0);

        /* Iniciar la transacción SPI */
        //Llama a la función spi_transceive() para realizar lel envio de los datos en tx_buffer al dispositivo SPI y esperará la respuesta
        // En spi_transceive:
        // spi_dev es el dispositivo SPI que se utilizará para la transacción
        // spi_cfg es la configuración del bus SPI, lo pasamos como dirección porque ya lo teníamos previamente configurado, miramos su posición en memoria
        // tx es la estructura spi_buf_set que contiene los buffers de datos para la transacción, idem que el otro, & para cogerlo de memoria
        // NULL indica que no se espera una respuesta del dispositivo esclavo
        int ret = spi_transceive(spi_dev, &spi_cfg, &tx, NULL);
        if (ret == 0) {                                
            // Si la transacción SPI se realiza correctamente (ret == 0), se entra al bloque if donde se imprimen los datos transmitidos

            for (int i = 0; i < BUFFER_SIZE; i++) {
                LOG_INF("%02X ", tx_buffer[i]);     // Recorre los datos, y los imprime en formato hexadecimal (de ahi el %02X)
            }
            tx_buffer[0] = tx_buffer[0] + 1;        // Modifica el primer byte del buffer (tx_buffer[0]). En este caso, incrementa
                                                    // el valor en 1 para enviar un dato diferente en la siguiente transacción SPI
        } else {
            printk("SPIM transaction failed: %d\n", ret);
            // Si hay un error, se imprime el mensaje de error con el código de retorn
        }

        /* Desactivar la señal CS */
        gpio_pin_set(gpio_dev, SPI_CS_PIN, 1);

        /* Esperar 2 segundos antes de la próxima transmisión */
        k_msleep(DELAY_MS); //k_msleep es una función de Zephyr que hace que el programa espere un número de milisegundos antes de continuar
    }
}