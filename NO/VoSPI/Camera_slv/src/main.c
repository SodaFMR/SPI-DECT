#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(spi_slave, LOG_LEVEL_INF);

#define SPI_DEV "SPI_3"

// Configuración SPI (Esclavo)
static const struct spi_config spi_cfg = {
    .frequency = 8000000,   // Máxima frecuencia SPI (8MHz)
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .slave = 1,             // Indicamos que es un esclavo
};

// Buffer para almacenar el frame
#define FRAME_WIDTH 128      // Ancho de la imagen (128px)
#define FRAME_HEIGHT 128     // Alto de la imagen (128px)
#define PIXEL_SIZE 2         // Tamaño de cada pixel en bytes (RGB565)
static uint8_t frame_buffer[FRAME_WIDTH * FRAME_HEIGHT * PIXEL_SIZE];

const struct device *spi_dev;

// Función para inicializar SPI en el esclavo
int spi_init(void) {
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi3));
    if (!device_is_ready(spi_dev)) {
        LOG_ERR("SPI no está listo");
        return -1;
    }
    LOG_INF("SPI esclavo inicializado");
    return 0;
}

// Función para simular la captura de un frame desde una cámara (aquí deberías llenar el buffer con datos reales)
int capturar_frame(void) {
    // Llenamos el buffer con datos simulados (por ejemplo, un color sólido o una imagen de prueba)
    for (int i = 0; i < sizeof(frame_buffer); i++) {
        frame_buffer[i] = (uint8_t)(i % 256);  // Simulando un patrón de colores
    }

    LOG_INF("Frame capturado (%d bytes)", sizeof(frame_buffer));
    return 0;
}

// Función para manejar la tarea SPI del esclavo
void spi_slave_task(void)
{
    if (spi_init() < 0) {
        return;
    }

    while (1) {
        // Simulamos la captura de un frame desde la cámara
        if (capturar_frame() == 0) {
            // Enviamos el frame capturado al maestro
            struct spi_buf tx_buf = { .buf = frame_buffer, .len = sizeof(frame_buffer) };
            struct spi_buf_set tx = { .buffers = &tx_buf, .count = 1 };

            int ret = spi_write(spi_dev, &spi_cfg, &tx);
            if (ret == 0) {
                LOG_INF("Frame enviado al maestro");
            } else {
                LOG_ERR("Error al enviar el frame");
            }
        }
        k_sleep(K_MSEC(33));  // 30 FPS
    }
}

void main(void)
{
    spi_slave_task();
}