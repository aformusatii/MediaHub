#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LGRemoteClient.h"

#include "Browser.h"
#include "Atmega128.h"
#include "HTPC.h"
#include "keymap.h"
#include "util.h"

typedef enum {
    MEDIAHUB = 1,
    SMART_TV = 2,
    HTPC = 3
} TargetDevice;

struct DeviceStatus {
    bool tvMouseOn;
    bool stopTvMouseMove;
};

void receivedData(unsigned char);
void afterBrowserInit(MouseCoordinates);
void closeWindow(void);
void mouseMove(MouseCoordinates);
void tvMouseMove(MouseCoordinates);
void mouseButtonPress(int);
void tvMouseButtonPress(int);
void mouseButtonRelease(int);
void keyPress(int);
void tvKeyPress(int);
void tvKeyPressSure(int);
void keyRelease(int);
void receivedUIEvent(char *);
void changeRemoteTarget(TargetDevice);

LGRemoteClient remoteClient;
Atmega atmega;
Browser* browser = Browser::getInstance();
HomeTheaterPC htpc;
TargetDevice remoteTarget = MEDIAHUB;
DeviceStatus deviceStatus;
MouseCoordinates oldMouseCoordinates;


int main(int argc, char** argv) {

    Configuration::getInstance()->parse(argc, argv);

    atmega.setReceivedDataHandler(receivedData);

    if (atmega.initAtmega()) {
        printf("Error initializing Atmega128.\n");
        return 1;
    }

    printf("Atmega128 successfully initialized.\n");
    fflush(stdout);

    if (remoteClient.init()) {
        printf("Error initializing LG Remote Client (CURL).\n");
        remoteClient.cleanup();
        return 1;
    }

    htpc.checkIfUp(1, 1);
    deviceStatus.tvMouseOn = false;
    deviceStatus.stopTvMouseMove = false;
    if (remoteClient.checkIfUp(1, 1)) {
        remoteClient.requestSession();
    }

    printf("LG Remote Client (CURL) successfully initialized.\n");
    fflush(stdout);

    browser->afterInit = afterBrowserInit;
    browser->closeWindow = closeWindow;
    browser->mouseMove = mouseMove;
    browser->mouseButtonPress = mouseButtonPress;
    browser->mouseButtonRelease = mouseButtonRelease;
    browser->keyPress = keyPress;
    browser->keyRelease = keyRelease;
    browser->receivedUIEvent = receivedUIEvent;

    // Hangs here in gtk main loop...
    browser->initBrowser(argc, argv);

    return 0;
}

void receivedData(unsigned char data) {
    DEBUG_LOG_ARG1("ATMEGA RECEIVED: [%d]\n", data);
    if (data == UART_IR_ON_OFF_RECEIVED) {
        remoteClient.setUpFlag(false);

        LG_KEY_DELAY;
        atmega.sendData(UART_IR_SEND_ON_OFF);
        sleep(2);

        // SMART_TV is ON! Make a connection with it...
        if (remoteClient.checkIfUp(10, 2)) {
            sleep(3);
            printf("TV is ON.");
            remoteClient.requestSession();
        }
    }
}

void mouseMove(MouseCoordinates mouseCoordinates) {
    switch (remoteTarget) {
        case SMART_TV:
            tvMouseMove(mouseCoordinates);
            break;

        case HTPC:
            if (htpc.isUp()) {
                htpc.mouseMove(mouseCoordinates.x, mouseCoordinates.y);
            }
            break;

        default:
            break;
    }
}

void tvMouseMove(MouseCoordinates mouseCoordinates) {
    int x = 0;
    int y = 0;
    int step = 4;

    if (remoteClient.isUp() && !deviceStatus.stopTvMouseMove && !oldMouseCoordinates.jump) {
        if (oldMouseCoordinates.x > mouseCoordinates.x) {
            x = -1 * step;
        } else if (oldMouseCoordinates.x < mouseCoordinates.x) {
            x = step;
        }

        if (oldMouseCoordinates.y > mouseCoordinates.y) {
            y = -1 * step;
        } else if (oldMouseCoordinates.y < mouseCoordinates.y) {
            y = step;
        }

        remoteClient.mouseMove(x, y);
        deviceStatus.tvMouseOn = true;
    }

    oldMouseCoordinates = mouseCoordinates;
}

void mouseButtonPress(int k) {

    switch (remoteTarget) {
        case SMART_TV:
            tvMouseButtonPress(k);
            break;

        case HTPC:
            if (htpc.isUp()) {
                htpc.mouseButtonPress(getHTPCMouseKey(k));
            }
            break;

        default:
            break;
    }

}

void tvMouseButtonPress(int k) {
    if (remoteClient.isUp() && (k == RIGHT_MOUSE || k == LEFT_MOUSE)) {
        deviceStatus.stopTvMouseMove = true;
        LG_KEY_DELAY;
        remoteClient.mouseClick();
        deviceStatus.stopTvMouseMove = false;
    }
}

void mouseButtonRelease(int k) {

    switch (remoteTarget) {
        case HTPC:
            if (htpc.isUp()) {
                htpc.mouseButtonRelease(getHTPCMouseKey(k));
            }
            break;

        default:
            break;
    }

}

void keyPress(int k) {

    DEBUG_LOG_ARG2("keyPress[%d][%c]\n", k, k);

    switch (k) {
        case MELE_KEY_MENU:
            switch (remoteTarget) {

                case MEDIAHUB:
                    changeRemoteTarget(SMART_TV);
                    return;

                case SMART_TV:
                    changeRemoteTarget(HTPC);
                    return;

                case HTPC:
                    changeRemoteTarget(MEDIAHUB);
                    return;

                default:
                    return;
            }
            break;


        case MELE_KEY_HOME:
            tvKeyPressSure(LG_KEY_EXTERNAL_INPUT);
            changeRemoteTarget(SMART_TV);
            return;
    }

    switch (remoteTarget) {
        case SMART_TV:
            tvKeyPress(k);
            break;

        case HTPC:
            if (htpc.isUp()) {
                htpc.keyPress(k);
            }
            break;

        default:
            break;
    }

}

void tvKeyPress(int k) {
    int tvKey = getTvKey(k);
    tvKeyPressSure(tvKey);
}

void tvKeyPressSure(int tvKey) {
    if (remoteClient.isUp() && tvKey > 0) {
        deviceStatus.stopTvMouseMove = true;
        if (deviceStatus.tvMouseOn) {
            remoteClient.sendEventWithMode("CursorVisible", "false", "Auto");
            LG_KEY_DELAY;
            deviceStatus.tvMouseOn = false;
        } else {
            LG_KEY_MIN_DELAY;
        }

        remoteClient.pressKeyTCP(tvKey);
        deviceStatus.stopTvMouseMove = false;
    }
}

void keyRelease(int k) {

    DEBUG_LOG_ARG2("keyRelease[%d][%c]\n", k, k);

    switch (k) {
        case MELE_KEY_MENU:
        case MELE_KEY_HOME:
            return;
    }

    /*if (isKeyWithOnlyRelease(k)) {
        keyPress(k);
    }*/

    switch (remoteTarget) {
        case HTPC:
            if (htpc.isUp()) {
                htpc.keyRelease(k);
            }
            break;

        default:
            break;
    }

}

void receivedUIEvent(char *eventName) {

    if (!remoteClient.isUp()) {
        return;
    }

    if (strcmp(eventName, "SELECT_SMART_TV") == 0) {

        tvKeyPressSure(LG_KEY_HOME_MENU);
        changeRemoteTarget(SMART_TV);

    } else if (strcmp(eventName, "SELECT_HTPC") == 0) {

        if (!htpc.checkIfUp(1,1)) {
            atmega.sendData(UART_SWITCH_ON_OFF);
            if (!htpc.checkIfUp(10,5)) {
                return;
            }
        }

        tvKeyPressSure(LG_KEY_EXTERNAL_INPUT);
        changeRemoteTarget(SMART_TV);

    } else if (strcmp(eventName, "EXIT") == 0) {

    }
}

void closeWindow() {
    printf("Close Windows.\n");
    fflush(stdout);
    remoteClient.cleanup();
    Configuration::getInstance()->cleanup();
}

void afterBrowserInit(MouseCoordinates mouseCoordinates) {
    oldMouseCoordinates = mouseCoordinates;
}

void changeRemoteTarget(TargetDevice device) {
    remoteTarget = device;

    switch (remoteTarget) {
        case MEDIAHUB:
            browser->setPreventDefaultEvents(false);
            browser->setMouseContinuousMove(false);
            break;

        case SMART_TV:
            browser->setPreventDefaultEvents(true);
            browser->setMouseContinuousMove(true);
            break;

        case HTPC:
            browser->setPreventDefaultEvents(true);
            browser->setMouseContinuousMove(false);
            break;

        default:
            break;
    }
}
