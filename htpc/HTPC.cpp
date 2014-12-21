#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#include "HTPC.h"
#include "Configuration.h"
#include "util.h"

volatile int sockfd = 0;
volatile bool _isHTPCUp = false;

int openSocket() {
    struct sockaddr_in serv_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        _isHTPCUp = false;
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(Configuration::getInstance()->htpcPort);

    if(inet_pton(AF_INET, Configuration::getInstance()->htpcHost, &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        _isHTPCUp = false;
        return 1;
    }

    struct timeval tv;

    tv.tv_sec = 2;  /* 30 Secs Timeout */
    tv.tv_usec = 0;  // Not init'ing this can cause strange errors

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       _isHTPCUp = false;
       return 1;
    }

    return 0;
}

int writeData(const char *data) {
    int n = 0;
    int respCount = 0;
    int retry = 1;
    char recvBuff[1024];
    char response[1024];

    if (sockfd == 0 && openSocket()) {
        return 1;
    }

    do {
        n = write(sockfd, data, strlen(data));

        //DEBUG_LOG_ARG1("Write[n=%d]\n", n);

        if (n > 0) {
            //DEBUG_LOG("Start read\n");
            while ((n = read(sockfd, recvBuff, sizeof(recvBuff))) > 0) {
                for (int i = 0; i < n; i++) {
                    char c = recvBuff[i];
                    response[respCount++] = c;
                    // read until new line character
                    if (c == '\n' || c == '\r') {
                        goto exit_read_loop;
                    }
                }
                //DEBUG_LOG_ARG1("[%d]\n", n);
            }

            exit_read_loop:

            response[respCount++] = 0;

            //DEBUG_LOG_ARG1("Read[n=%d]\n", n);

            if(n > -1 && strcmp("OK\n", response) != 0 && strcmp("OK\r", response) != 0) {
                n = -1;
                fprintf(stdout, "[HTPC.writeData] Response: [%s]\n", response);
                break;
            }
        }

        if(n < 0) {
            close(sockfd);
            openSocket();
        } else {
            retry = 0;
        }

        fflush(stdout);

    } while (retry--);

    return (n < 0) ? 1 : 0;
}

void HomeTheaterPC::mouseMove(int x, int y) {
    char data[50];
    memset(data, 0,sizeof(data));
    sprintf(data, "MOUSE_MOVE %d %d\n", x, y);

    if (writeData(data)) {
        fprintf(stdout, "[HTPC.mouseMove] error at data write.\n");
        fflush(stdout);
    }
}

void HomeTheaterPC::mouseButtonPress(int key) {
    char data[50];
    memset(data, 0,sizeof(data));
    sprintf(data, "MOUSE_BUTTON_PRESS %d\n", key);

    if (writeData(data)) {
        fprintf(stdout, "[HTPC.mouseButtonPress] error at data write.\n");
        fflush(stdout);
    }
}

void HomeTheaterPC::mouseButtonRelease(int key) {
    char data[50];
    memset(data, 0,sizeof(data));
    sprintf(data, "MOUSE_BUTTON_RELEASE %d\n", key);

    if (writeData(data)) {
        fprintf(stdout, "[HTPC.mouseButtonRelease] error at data write.\n");
        fflush(stdout);
    }
}

void HomeTheaterPC::keyPress(int key) {
    char data[50];
    memset(data, 0,sizeof(data));
    sprintf(data, "KEY_PRESS %d\n", key);

    if (writeData(data)) {
        fprintf(stdout, "[HTPC.keyPress] error at data write.\n");
        fflush(stdout);
    }
}

void HomeTheaterPC::keyRelease(int key) {
    char data[50];
    memset(data, 0,sizeof(data));
    sprintf(data, "KEY_RELEASE %d\n", key);

    if (writeData(data)) {
        fprintf(stdout, "[HTPC.keyRelease] error at data write.\n");
        fflush(stdout);
    }
}

bool HomeTheaterPC::checkIfUp(int retries, int timeout) {
    _isHTPCUp = checkIfHostIsUp(Configuration::getInstance()->htpcHost, retries, timeout);
    return _isHTPCUp;
}

bool HomeTheaterPC::isUp() {
    return _isHTPCUp;
}

void HomeTheaterPC::setUpFlag(bool value) {
    _isHTPCUp = value;
}
