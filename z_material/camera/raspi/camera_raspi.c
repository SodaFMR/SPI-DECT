#include <stdio.h>
#include <bcm2835.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>


// Definiciones de la cámara
#define CAMERA_WIDTH 1280
#define CAMERA_HEIGHT 720
#define MAX_BYTES_TO_PRINT 100  // Máximo número de bytes que se mostrarán en consola
#define DEVICE "/dev/video0"

// Definir los pines GPIO
#define SCLK_PIN RPI_V2_GPIO_P1_19  // GPIO 10 (SCLK)
#define MISO_PIN RPI_V2_GPIO_P1_11  // GPIO 17 (MISO)
#define CS_PIN   RPI_V2_GPIO_P1_15  // GPIO 22 (CS)



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
        uint8_t bit = (byte >> i) & 0x01;
        send_bit(bit);
        while (check_pin_state(SCLK_PIN));
        while (!check_pin_state(SCLK_PIN));
        //while (check_pin_state(SCLK_PIN));  // Espera a que el reloj baje. Esta a 0 y pasa a 1
        //while (!check_pin_state(SCLK_PIN));   // Espera a que el reloj suba
    }
}



struct buffer {
    void   *start;
    size_t length;
};

int init() {
    int fd = open(DEVICE, O_RDWR);
    if (fd == -1) {
        perror("Error al abrir el dispositivo de video");
        return 0;
    }

    // Inicializar la librería bcm2835
    if (!bcm2835_init()) {
        printf("Error al inicializar bcm2835!\n");
        return 0;
    }

    // Configurar los pines como entradas
    bcm2835_gpio_fsel(SCLK_PIN, BCM2835_GPIO_FSEL_INPT);  // SCLK como entrada
    bcm2835_gpio_fsel(MISO_PIN, BCM2835_GPIO_FSEL_OUTP);  // MISO como entrada
    bcm2835_gpio_fsel(CS_PIN, BCM2835_GPIO_FSEL_INPT);    // CS como entrada

    // Solicitar capacidad del dispositivo
    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("Error al consultar las capacidades del dispositivo");
        return 0;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "El dispositivo no admite captura de video\n");
        return 0;
    }

    // Configuración de formato de captura
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = CAMERA_WIDTH;
    fmt.fmt.pix.height = CAMERA_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;  // Formato JPEG
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Error al configurar el formato de video");
        return 0;
    }

    // Solicitar buffers
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Error al solicitar buffers");
        return 0;
    }

    struct buffer buffer;
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
        perror("Error al consultar el buffer");
        return 0;
    }

    buffer.length = buf.length;
    buffer.start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

    if (buffer.start == MAP_FAILED) {
        perror("Error al mapear el buffer");
        return 0;
    }

    // Activar captura de video
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Error al activar el streaming");
        return 0;
    }

    // Capturar un solo frame
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Error al encolar el buffer");
        return 0;
    }

    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Error al desenfilar el buffer");
        return 0;
    }

    // Imprimir los primeros bytes capturados
    printf("Bytes de la imagen capturada (primeros %d bytes):\n", MAX_BYTES_TO_PRINT);
    for (int i = 0; i < buf.bytesused && i < MAX_BYTES_TO_PRINT; i++) {
        printf("%02X ", ((unsigned char*)buffer.start)[i]);
    }
    printf("\n");

    // Guardar la imagen en un archivo JPEG
    FILE *file = fopen("captura_imagen.jpeg", "wb");
    if (file) {
        fwrite(buffer.start, buf.bytesused, 1, file);
        fclose(file);
        printf("Imagen guardada como captura_imagen.jpeg\n");
    }

    printf("Esperando comunicación SPI 1...\n");

    // Bucle para verificar continuamente el estado de los pines
    while (1) {
        if(check_pin_state(CS_PIN) != 1) {
            // printf("CS está en BAJO (LOW), se está realizando una comunicación SPI\n");
            send_byte(0xaa);  // Enviar un byte de prueba
        };

        // Pausar por 1 segundo antes de verificar nuevamente
        //bcm2835_delay(1);  // Pausa de 1 segundo
    }

    // Limpiar los pines GPIO al salir
    bcm2835_close();

    // Limpiar recursos
    munmap(buffer.start, buffer.length);
    close(fd);

    return 0;
}
