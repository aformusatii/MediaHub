
#define DEV_ENV false

#if defined(DEV_ENV) & DEV_ENV
    #define DEBUG_LOG(STR) fprintf(stdout, STR); fflush(stdout);
    #define DEBUG_LOG_ARG1(STR, VAL1) fprintf(stdout, STR, VAL1); fflush(stdout);
    #define DEBUG_LOG_ARG2(STR, VAL1, VAL2) fprintf(stdout, STR, VAL1, VAL2); fflush(stdout);

    #define LG_HOST "localhost"
    #define LG_TCP_PORT 8080
    #define LG_UDP_PORT 7070
    #define LG_PAIRING_KEY 990905

    #define HTPC_HOST "192.168.1.117"
    #define HTPC_TCP_PORT 5555
#else
    #define DEBUG_LOG(STR)
    #define DEBUG_LOG_ARG1(STR, VAL1)
    #define DEBUG_LOG_ARG2(STR, VAL1, VAL2)

    #define LG_HOST "192.168.1.230"
    #define LG_TCP_PORT 8080
    #define LG_UDP_PORT 7070
    #define LG_PAIRING_KEY 990905

    #define HTPC_HOST "192.168.1.115"
    #define HTPC_TCP_PORT 5555
#endif

// Configuration class
class Configuration {
    private:
        Configuration() {
            host = LG_HOST;
            port = LG_TCP_PORT;
            udpPort = LG_UDP_PORT;
            pairingKey = LG_PAIRING_KEY;
            requestPairingKey = false;
            htpcHost = HTPC_HOST;
            htpcPort = HTPC_TCP_PORT;
        };

        Configuration(Configuration const&) {
            host = LG_HOST;
            port = LG_TCP_PORT;
            udpPort = LG_UDP_PORT;
            pairingKey = LG_PAIRING_KEY;
            requestPairingKey = false;
            htpcHost = HTPC_HOST;
            htpcPort = HTPC_TCP_PORT;
        };

        static Configuration* configInstance;

    public:
        static Configuration* getInstance();
        const char *host;
        int port;
        int udpPort;
        int pairingKey;
        bool requestPairingKey;
        const char *htpcHost;
        int htpcPort;

        void cleanup();
        void parse(int argc, char **argv);
};
