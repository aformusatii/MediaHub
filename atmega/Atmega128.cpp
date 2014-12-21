#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "Atmega128.h"
#include "env.h"

#if defined(RASPBERRY_PI) & RASPBERRY_PI
    #include <wiringPi.h>
#endif

static int serialPort;
static void (*receivedData)(unsigned char);

/**
 * http://en.wikibooks.org/wiki/Serial_Programming/termios
 */
int Atmega::initAtmega() {

    #if defined(RASPBERRY_PI) & RASPBERRY_PI
        serialPort = open("/dev/ttyAMA0", O_RDWR );
    #else
        serialPort = open("/dev/ttyUSB0", O_RDWR );
    #endif



    if (serialPort < 0) {
        fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
        return 1;
    }

    struct termios options ;
    memset(&options,0,sizeof(options));

    tcgetattr (serialPort, &options); // Read current options
    options.c_cflag &= ~CSIZE;        // Mask out size, disable Parity
    options.c_cflag |= CS8 ;          // Or in 8-bits
    options.c_cflag |= PARENB ;       // Or in 8-bits
    options.c_cflag |= PARODD ;       // parity for input and output is odd
    options.c_cflag |= CSTOPB ;       // Set two stop bits, rather than one.

    options.c_iflag=0;
    options.c_oflag=0;
    options.c_lflag=0;
    options.c_cc[VMIN]=1;
    options.c_cc[VTIME]=5;

    if(cfsetispeed(&options, B38400) < 0 || cfsetospeed(&options, B38400) < 0) {
        fprintf (stderr, "Unable set UART configuration/speed: %s\n", strerror (errno)) ;
        return 1 ;
    }

    if(tcsetattr(serialPort, TCSAFLUSH, &options) < 0) {
        fprintf (stderr, "Unable set UART configuration: %s\n", strerror (errno)) ;
        return 1 ;
    }

    #if defined(RASPBERRY_PI) & RASPBERRY_PI
        if (wiringPiSetup () < 0) {
          fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
          return 1 ;
        }

        pinMode (W_GPIO_22, OUTPUT);
        pinMode (W_GPIO_17, OUTPUT);

        pinModeAlt (W_GPIO_15_UART_RXD, WPI_MODE_PIFACE);

        digitalWrite (W_GPIO_22, HIGH);
        digitalWrite (W_GPIO_17, HIGH);
    #endif

    pthread_t p_id; // thread object
    pthread_create(&p_id, NULL, Atmega::serialRead, NULL); // create it

    return 0;
}

void Atmega::setReceivedDataHandler(void (*handler)(unsigned char)) {
    receivedData = handler;
}

void* Atmega::serialRead(void *rt) {
    unsigned char c = 0;

    for (;;) {
        int r = read(serialPort, &c, 1);
        if (r > 0) {
            receivedData(c);
        }
    }

    pthread_exit(NULL);
}

void Atmega::sendData(unsigned char data) {
    int n = write(serialPort, &data, 1);
    if (n < 0) {
        printf("[Atmega] error on sendData.") ;
        fflush (stdout);
    }
}
