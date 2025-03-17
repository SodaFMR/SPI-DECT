#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

LOG_MODULE_REGISTER(spi_master, LOG_LEVEL_INF);

#define SPI_DEV "SPI_3"

// Definir configuración SPI
static const struct spi_config spi_cfg = {
    .frequency = 8000000,  // Máximo de 8 MHz
    .operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
    .slave = 0,            // Maestro
};

// Buffer para recibir el frame
#define FRAME_WIDTH 128
#define FRAME_HEIGHT 128
#define PIXEL_SIZE 2 // Suponiendo RGB565
static uint8_t rx_buffer[FRAME_WIDTH * FRAME_HEIGHT * PIXEL_SIZE];

// Inicializar la pantalla ILI9163
static const struct device *disp_dev;

void init_display(void)
{
    disp_dev = device_get_binding("TFT_ILI9163");  // Asegúrate de que el nombre es correcto en tu configuración
    if (disp_dev == NULL) {
        LOG_ERR("No se pudo obtener el dispositivo de la pantalla");
        return;
    }

    struct display_capabilities cap;
    display_get_capabilities(disp_dev, &cap);

    if (cap.x_resolution != FRAME_WIDTH || cap.y_resolution != FRAME_HEIGHT) {
        LOG_ERR("Resolución de la pantalla no coincide");
        return;
    }

    // Inicializar la pantalla y limpiarla
    display_clear(disp_dev, COLOR_BLACK);
    LOG_INF("Pantalla inicializada.");
}

// Función para mostrar el frame recibido en la pantalla
void display_frame(uint8_t *frame)
{
    struct display_buffer_descriptor desc = {
        .width = FRAME_WIDTH,
        .height = FRAME_HEIGHT,
        .pitch = FRAME_WIDTH * PIXEL_SIZE,
        .buf = frame,
    };

    int ret = display_write(disp_dev, &desc, 0, 0);
    if (ret != 0) {
        LOG_ERR("Error al mostrar el frame");
    }
}

void spi_master_task(void)
{
    const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi3));
    if (!device_is_ready(spi_dev)) {
        LOG_ERR("SPI no está listo");
        return;
    }

    struct spi_buf rx_buf = { .buf = rx_buffer, .len = sizeof(rx_buffer) };
    struct spi_buf_set rx = { .buffers = &rx_buf, .count = 1 };

    // Inicializar la pantalla
    init_display();

    while (1) {
        int ret = spi_read(spi_dev, &spi_cfg, &rx);
        if (ret == 0) {
            LOG_INF("Frame recibido (%d bytes)", sizeof(rx_buffer));
            // Mostrar el frame en la pantalla
            display_frame(rx_buffer);
        } else {
            LOG_ERR("Error en la lectura SPI");
        }

        k_sleep(K_MSEC(33)); // Aproximadamente 30 FPS
    }
}

void main(void)
{
    // Ejecutar la tarea SPI
    spi_master_task();
}