#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "zlib.h"
#include "util.h"

#include "LGRemoteClient.h"

volatile bool _isTVUp = false;

int LGRemoteClient::init() {
    int result = curl_global_init(CURL_GLOBAL_ALL);

    if (result != 0) {
        // If this function returns non-zero, something went wrong and you cannot use the other curl functions.
        return 1;
    }

    xmlHeaders = NULL;
    xmlHeaders = curl_slist_append(xmlHeaders, "Content-Type: application/atom+xml");

    initUDPSocket();

    return 0;
}

void LGRemoteClient::cleanup() {
    curl_slist_free_all(xmlHeaders);
    curl_global_cleanup();

    close(sockfd);
}

void LGRemoteClient::initUDPSocket() {
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_aton(Configuration::getInstance()->host, &servaddr.sin_addr);
    servaddr.sin_port = htons(Configuration::getInstance()->udpPort);
}

void LGRemoteClient::requestSession() {
    LGRemoteClient::Response response = sendXMLPostRequest(PATH_AUTH_REQ, XML_AUTH_REQ, Configuration::getInstance()->pairingKey);

    if (checkResponseStatus(response.content, false)) {
        char *sessionIdStr = substring(response.content, "<session>", "</session>");

        if (sessionIdStr) {
            sessionId = atol(sessionIdStr);
            free(sessionIdStr);
        }

    } else {
        printf("[requestSession] ROAPError: %s", response.content);
    }

    free(response.content);
}

void LGRemoteClient::requestPairingKey() {
    LGRemoteClient::Response response = sendXMLPostRequest(PATH_AUTH_KEY_REQ, XML_AUTH_KEY_REQ);

    if (checkResponseStatus(response.content, false)) {
        printf("Yout should see PairingKey on TV display.");
    } else {
        printf("[requestPairingKey] ROAPError: %s", response.content);
    }

    free(response.content);
}

bool LGRemoteClient::checkResponseStatus(const char *content, bool requestSessionOn401Error) {
    char* roapError = substring(content, "<ROAPError>", "</ROAPError>");
    bool result = false;
    if (roapError) {
        result = (strcmp(roapError, "200") == 0);
        if (requestSessionOn401Error && !result && (strcmp(roapError, "401") == 0)) {
            requestSession();
        }
        free(roapError);
    }
    return result;
}

size_t LGRemoteClient::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct LGRemoteClient::Response *mem = (struct LGRemoteClient::Response *) userp;

    mem->content = (char *) realloc(mem->content, mem->size + realsize + 1);
    if (mem->content == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->content[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->content[mem->size] = 0;

    return realsize;
}

LGRemoteClient::Response LGRemoteClient::sendXMLPostRequest(const char *urlPath, const char *xmlContent, ...) {
    LGRemoteClient::Response response;
    response.content = (char *) malloc(1); /* will be grown as needed by the realloc above */
    response.size = 0; /* no data at this point */

    va_list argptr;
    va_start(argptr, xmlContent);
    char postData[sizeof(xmlContent) + 500];
    vsprintf(postData, xmlContent, argptr);

    char absoluteUrlPath[100];
    memset(absoluteUrlPath, 0,sizeof(absoluteUrlPath));
    sprintf(absoluteUrlPath, "http://%s:%d%s", Configuration::getInstance()->host, Configuration::getInstance()->port, urlPath);

    CURL *curl = curl_easy_init();

    /* pass our list of custom made headers */
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, xmlHeaders);

    /* First set the URL that is about to receive our POST. This URL can
     just as well be a https:// URL if that is what should receive the
     data. */
    curl_easy_setopt(curl, CURLOPT_URL, absoluteUrlPath);

    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * ) &response);

    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);

    /* Switch on full protocol/debug output while testing */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, USR_CURLOPT_VERBOSE);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
    //curl_easy_setopt(curl, CURLOPT_USERAGENT, USR_CURLOPT_USERAGENT);

    /* make connection get closed at once after use */
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, USR_CURLOPT_FORBID_REUSE);

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, USR_CURLOPT_FOLLOWLOCATION);

    /* set maximum time the request is allowed to take  */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

    /* timeout for the connect phase  */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);

    /* Perform the request, res will get the return code */
    response.res = curl_easy_perform(curl);

    /* Check for errors */
    if (response.res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(response.res));
        _isTVUp = false;
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    return response;
}

LGRemoteClient::Response LGRemoteClient::sendGetRequest(const char *urlPath) {
    LGRemoteClient::Response response;
    response.content = (char *) malloc(1); /* will be grown as needed by the realloc above */
    response.size = 0; /* no data at this point */

    char absoluteUrlPath[100];
    memset(absoluteUrlPath, 0,sizeof(absoluteUrlPath));
    sprintf(absoluteUrlPath, "http://%s:%d%s", Configuration::getInstance()->host, Configuration::getInstance()->port, urlPath);

    CURL *curl = curl_easy_init();

    /* pass our list of custom made headers */
    //curl_easy_setopt(curl, CURLOPT_HTTPHEADER, xmlHeaders);
    /* First set the URL that is about to receive our POST. This URL can
     just as well be a https:// URL if that is what should receive the
     data. */
    curl_easy_setopt(curl, CURLOPT_URL, absoluteUrlPath);

    /* send all data to this function  */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * ) &response);

    /* Switch on full protocol/debug output while testing */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, USR_CURLOPT_VERBOSE);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
    //curl_easy_setopt(curl, CURLOPT_USERAGENT, USR_CURLOPT_USERAGENT);

    /* make connection get closed at once after use */
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, USR_CURLOPT_FORBID_REUSE);

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, USR_CURLOPT_FOLLOWLOCATION);

    /* set maximum time the request is allowed to take  */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

    /* timeout for the connect phase  */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);

    /* Perform the request, res will get the return code */
    response.res = curl_easy_perform(curl);

    /* Check for errors */
    if (response.res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(response.res));
        _isTVUp = false;
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    return response;
}

int addTwoBytes(uint8_t *packet, int index, uint16_t value) {
   packet[index++] = (uint8_t) (value);
   packet[index++] = (uint8_t) (value >> 8);
   return index;
}

int addFourBytes(uint8_t *packet, int index, uint32_t value) {
   packet[index++] = (uint8_t) (value);
   packet[index++] = (uint8_t) (value >> 8);
   packet[index++] = (uint8_t) (value >> 16);
   packet[index++] = (uint8_t) (value >> 24);
   return index;
}

/*  UDP packets captured with wireshark.
# Just type "udp.port == 7070" into the filter box.

# <------------   UDP Payload 18, 22, or 26 bytes   -------------->
# 1) Original Message
# uint32      uint32     uint16 uint32      uint32      uint32      uint32
# <---------> <---------> <---> <---------> <---------> <---------> <--------->
# 00:00:00:00 54:13:43:65 02:00 08:00:00:00 00:00:00:00 04:00:00:00 04:00:00:00
# <---------> <---------> <---> <---------> <---------> <---------> <--------->
# Zero-pad    session     cmd1  cmd2        data1       data2*      data3*
#
# * The data2 and data3 are optional extra arguments

# Each *individual* fields are little endian (LSB first --> MSB last)
# Its not as simple as the expected network native big endian.
# We must reverse each individual field from the Network order.
# So for example cmd1 "02:00" is actually (uint16)0x02
#           and  cmd2 "08:00:00:00"   ==  (uint32)0x08

# 2) Final Message with crc32 checksum filled in.
#    Where "crc32" field = crc32() of zero-padded Message 1) above
# 03:14:6b:6d 54:13:43:65 02:00 08:00:00:00 00:00:00:00 04:00:00:00
# <---------> <---------> <---> <---------> <---------> <--------->
# crc32       session     cmd1  cmd2        data1       data2*
#
# Final UDP packet
# "03:14:6b:6d:54:13:43:65:02:00:08:00:00:00 00:00:00:00 04:00:00:00" */
int LGRemoteClient::sendUDPRequest(uint64_t cmd, int64_t data1, int64_t data2, int64_t data3) {

   uint16_t cmd1 = (uint16_t) (cmd >> 32);
   uint32_t cmd2 = (uint32_t) cmd;

   uint32_t sessionId = (uint32_t) this->sessionId;

   int datac = (data1 < NO_VALUE ? 1 : 0) + (data2 < NO_VALUE ? 1 : 0) + (data3 < NO_VALUE ? 1 : 0);

   int len = 4 * (3 + datac) + 2;

   uint8_t *packet = (uint8_t *) malloc(len);

   int i = 0;
   i = addFourBytes(packet, i, 0);
   i = addFourBytes(packet, i, sessionId);
   i = addTwoBytes(packet, i, cmd1);
   i = addFourBytes(packet, i, cmd2);

   if (data1 < NO_VALUE) {
       i = addFourBytes(packet, i, (uint32_t) data1);
   }

   if (data2 < NO_VALUE) {
       i = addFourBytes(packet, i, (uint32_t) data2);
   }

   if (data3 < NO_VALUE) {
       i = addFourBytes(packet, i, (uint32_t) data3);
   }

   uint32_t crc = crc32(0L, packet, len);

   addFourBytes(packet, 0, crc);

   /* for (i = 0; i < len; i++) {
       printf("%x:", packet[i]);
   }*/

   sendto(sockfd,
          packet,
          len,
          0,
          (struct sockaddr *)&servaddr,
          sizeof(servaddr));

   free(packet);

   fflush(stdout);

   return 0;
}

void LGRemoteClient::mouseMove(uint32_t x, uint32_t y) {
    sendUDPRequest(CMD_MOUSE_MOVE, x, y, NO_VALUE);
    usleep(50);
}

void LGRemoteClient::mouseClick() {
    LGRemoteClient::Response response = sendXMLPostRequest(PATH_GENERIC_COMMAND, XML_GENERIC_COMMAND, "HandleTouchClick" , "20");

    if (!checkResponseStatus(response.content, true)) {
        printf("[mouseClick] ROAPError: %s", response.content);
    }

    free(response.content);
}

void LGRemoteClient::pressKeyUDP(uint32_t key) {
    sendUDPRequest(CMD_PRESS_KEY, key, NO_VALUE, NO_VALUE);
    usleep(50);
}

void LGRemoteClient::pressKeyTCP(uint32_t key) {
    LGRemoteClient::Response response = sendXMLPostRequest(PATH_HANDLE_KEY_INPUT, XML_HANDLE_KEY_INPUT, key);

    if (!checkResponseStatus(response.content, true)) {
        printf("[pressKeyTCP] ROAPError: %s", response.content);
    }

    free(response.content);
}

void LGRemoteClient::sendEventWithMode(const char *event, const char *value, const char *mode) {
    LGRemoteClient::Response response = sendXMLPostRequest(PATH_SEND_EVENT_WITH_MODE, XML_SEND_EVENT_WITH_MODE, event, value, mode);

    if (!checkResponseStatus(response.content, true)) {
        printf("[sendEventWithMode] ROAPError: %s for %s", response.content, event);
    }

    free(response.content);
}

void LGRemoteClient::sendEventWithState(const char *event, const char *value, const char *state) {
    LGRemoteClient::Response response = sendXMLPostRequest(PATH_SEND_EVENT_WITH_STATE, XML_SEND_EVENT_WITH_STATE, event, value, state);

    if (!checkResponseStatus(response.content, true)) {
        printf("[sendEventWithState] ROAPError: %s for %s", response.content, event);
    }

    free(response.content);
}

void LGRemoteClient::sendCommand(const char *command, const char *param) {
    LGRemoteClient::Response response = sendXMLPostRequest(PATH_GENERIC_COMMAND, XML_GENERIC_COMMAND, command, param);

    if (!checkResponseStatus(response.content, true)) {
        printf("[sendCommand] ROAPError: %s for %s", response.content, command);
    }

    free(response.content);
}

/**
 * timeout - in seconds
 */
bool LGRemoteClient::checkIfUp(int retries, int timeout) {
    _isTVUp = checkIfHostIsUp(Configuration::getInstance()->host, retries, timeout);
    return _isTVUp;
}

bool LGRemoteClient::isUp() {
    return _isTVUp;
}

void LGRemoteClient::setUpFlag(bool value) {
    _isTVUp = value;
}

