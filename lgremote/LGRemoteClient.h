#include <stdint.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <curl/curl.h>

#include "Configuration.h"

#define LG_KEY_DELAY usleep(500000)
#define LG_KEY_MIN_DELAY usleep(150000)

// Request Pairing Key
static const char PATH_AUTH_KEY_REQ[] = "/roap/api/auth";
static const char XML_AUTH_KEY_REQ[] = "<?xml version=\"1.0\" encoding=\"utf-8\"?><auth><type>AuthKeyReq</type></auth>";

// Request Session
static const char PATH_AUTH_REQ[] = "/roap/api/auth";
static const char XML_AUTH_REQ[] = "<auth><name>AuthReq</name><value>%d</value></auth>";

// Handle Key Input
static const char PATH_HANDLE_KEY_INPUT[] = "/roap/api/command";
static const char XML_HANDLE_KEY_INPUT[] = "<command><name>HandleKeyInput</name><value>%d</value></command>";

// Send Event With Mode
static const char PATH_SEND_EVENT_WITH_MODE[] = "/roap/api/event";
static const char XML_SEND_EVENT_WITH_MODE[] = "<event><name>%s</name><value>%s</value><mode>%s</mode></event>";

// Send Event With State
static const char PATH_SEND_EVENT_WITH_STATE[] = "/roap/api/event";
static const char XML_SEND_EVENT_WITH_STATE[] = "<event><name>%s</name><state>%s</state><value>%s</value></event>";

// Send Command
static const char PATH_GENERIC_COMMAND[] = "/roap/api/command";
static const char XML_GENERIC_COMMAND[] = "<command><name>%s</name><value>%s</value></command>";

#define CMD_MOUSE_MOVE  0x000200000008
#define CMD_PRESS_KEY   0x000100000004

#define NO_VALUE 0x100000000

#define DISABLED_OPT 0L
#define ENABLED_OPT  1L

// curl configuration
#define USR_CURLOPT_VERBOSE DISABLED_OPT
#define USR_CURLOPT_USERAGENT "libcurl-agent/1.0"
#define USR_CURLOPT_FORBID_REUSE ENABLED_OPT
#define USR_CURLOPT_FOLLOWLOCATION ENABLED_OPT

// LGRemoteClient class
class LGRemoteClient {
    private:
        struct curl_slist *xmlHeaders;
        long sessionId;

        int sockfd;
        struct sockaddr_in servaddr;

        bool checkResponseStatus(const char *, bool);

    public:
        struct Response {
            CURLcode res;
            char* content;
            size_t size;
        };

        int init();
        void cleanup();

        void initUDPSocket();

        void requestSession();
        void requestPairingKey();

        static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
        Response sendXMLPostRequest(const char *, const char *, ...);
        Response sendGetRequest(const char *);

        int sendUDPRequest(uint64_t cmd, int64_t data1, int64_t data2, int64_t data3);

        void mouseMove(uint32_t, uint32_t);
        void mouseClick();
        void pressKeyUDP(uint32_t);
        void pressKeyTCP(uint32_t);
        void sendEventWithMode(const char *, const char *, const char *);
        void sendEventWithState(const char *, const char *, const char *);
        void sendCommand(const char *, const char *);
        bool checkIfUp(int, int);
        bool isUp();
        void setUpFlag(bool);
};
