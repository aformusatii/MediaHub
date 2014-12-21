#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "Configuration.h"

Configuration* Configuration::configInstance = NULL;

Configuration* Configuration::getInstance() {
    if (!configInstance) {
        configInstance = new Configuration;
    }

    return configInstance;
}

/**
 * http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
 */
void Configuration::parse(int argc, char **argv) {
    int c;

    while (1) {
        static struct option long_options[] = {
            /* These options don't set a flag.
             We distinguish them by their indices. */
            { "lgTvHost",              required_argument, 0, 'a' },
            { "lgTvTCPPort",           required_argument, 0, 'b' },
            { "lgTvUDPPort",           required_argument, 0, 'c' },
            { "lgTVPairingKey",        required_argument, 0, 'd' },
            { "lgTVRequestPairingKey", no_argument,       0, 'e' },

            { "htpcHost",              required_argument, 0, 'f' },
            { "htpcTCPPort",           required_argument, 0, 'g' },

            { 0, 0, 0, 0 }
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "a:b:c:d:e:f:g;", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 'a':
                host = optarg;
                break;

            case 'b':
                port = atoi(optarg);
                break;

            case 'c':
                udpPort = atoi(optarg);
                break;

            case 'd':
                pairingKey = atoi(optarg);
                break;

            case 'e':
                requestPairingKey = true;
                break;

            case 'f':
                htpcHost = optarg;
                break;

            case 'g':
                htpcPort = atoi(optarg);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                abort();
        }
    }

    printf("Using configuration: \nLG Host: %s\nLG TCP Port: %d\nLG UDP Port: %d\nLG Pairing Key: %d\nLG Pairing Key Request?: %d\nHTPC Host: %s\nHTPC Port: %d\n",
            host, port, udpPort, pairingKey, requestPairingKey, htpcHost, htpcPort);
    fflush(stdout);
}

void Configuration::cleanup() {
}
