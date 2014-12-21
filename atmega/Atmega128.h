#define UART_IR_ON_OFF_RECEIVED 10
#define UART_SWITCH_ON_OFF      11
#define UART_IR_SEND_ON_OFF     12

/**
+----------+-Rev2-+------+--------+------+-------+
| wiringPi | GPIO | Phys | Name   | Mode | Value |
+----------+------+------+--------+------+-------+
|      0   |  17  |  11  | GPIO 0 | IN   | Low   |
|      1   |  18  |  12  | GPIO 1 | IN   | Low   |
|      2   |  27  |  13  | GPIO 2 | IN   | Low   |
|      3   |  22  |  15  | GPIO 3 | IN   | Low   |
|      4   |  23  |  16  | GPIO 4 | OUT  | High  |
|      5   |  24  |  18  | GPIO 5 | IN   | Low   |
|      6   |  25  |  22  | GPIO 6 | IN   | Low   |
|      7   |   4  |   7  | GPIO 7 | IN   | Low   |
|      8   |   2  |   3  | SDA    | ALT0 | High  |
|      9   |   3  |   5  | SCL    | ALT0 | High  |
|     10   |   8  |  24  | CE0    | ALT0 | High  |
|     11   |   7  |  26  | CE1    | ALT0 | High  |
|     12   |  10  |  19  | MOSI   | ALT0 | Low   |
|     13   |   9  |  21  | MISO   | ALT0 | Low   |
|     14   |  11  |  23  | SCLK   | ALT0 | Low   |
|     15   |  14  |   8  | TxD    | ALT0 | High  |
|     16   |  15  |  10  | RxD    | ALT0 | Low   |
|     17   |  28  |   3  | GPIO 8 | IN   | Low   |
|     18   |  29  |   4  | GPIO 9 | IN   | Low   |
|     19   |  30  |   5  | GPIO10 | IN   | Low   |
|     20   |  31  |   6  | GPIO11 | IN   | Low   |
+----------+------+------+--------+------+-------+
**/

#define W_GPIO_17            0
#define W_GPIO_18_PCM_CLK    1
#define W_GPIO_27            2
#define W_GPIO_22            3
#define W_GPIO_23            4
#define W_GPIO_24            5
#define W_GPIO_25            6
#define W_GPIO_4_GPCLK       7
#define W_GPIO_2_I2C_SDA     8
#define W_GPIO_3_I2C_SCL     9
#define W_GPIO_8_SPI_CE0_N   10
#define W_GPIO_7_SPI_CE1_N   11
#define W_GPIO_10_SPI_MOSI   12
#define W_GPIO_9_SPI_MISO    13
#define W_GPIO_11_SPI_SCLK   14
#define W_GPIO_14_UART_TXD   15
#define W_GPIO_15_UART_RXD   16

// Browser class
class Atmega {
    public:
        static void* serialRead(void *);
        int initAtmega();
        void sendData(unsigned char);
        void setReceivedDataHandler(void (*receivedData)(unsigned char));
};
