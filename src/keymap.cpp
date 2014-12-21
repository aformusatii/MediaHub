#include "LGKeymap.h"
#include "keymap.h"
#include "HTPCKeymap.h"

int getTvKey(int pcKey) {

    switch (pcKey) {
        case MELE_KEY_OK:
            return LG_KEY_OK;
        case MELE_KEY_UP:
            return LG_KEY_UP_KEY;
        case MELE_KEY_DOWN:
            return LG_KEY_DOWN_KEY;
        case MELE_KEY_LEFT:
            return LG_KEY_LEFT_KEY;
        case MELE_KEY_RIGHT:
            return LG_KEY_RIGHT_KEY;
        case MELE_KEY_HOME:
            return LG_KEY_HOME_MENU;
        case MELE_KEY_VOL_UP:
            return LG_KEY_VOLUME_UP;
        case MELE_KEY_VOL_DOWN:
            return LG_KEY_VOLUME_DOWN;
        case MELE_KEY_FORWARD:
            return LG_KEY_SKIP_FORWARD;
        case MELE_KEY_BACK:
            return LG_KEY_SKIP_BACKWARD;
        case MELE_KEY_ESC:
            return LG_KEY_PREVIOUS_KEY_BACK;
        default:
            break;
    }

    return -1;
}

int getHTPCMouseKey(int key) {
    switch (key) {
        case RIGHT_MOUSE:
            return BUTTON1_MASK;
        case LEFT_MOUSE:
            return BUTTON3_MASK;
        default:
            break;
    }

    return -1;
}


int getHTPCKey(int key) {
    switch (key) {
        case MELE_KEY_OK:
            return LG_KEY_OK;
        case MELE_KEY_UP:
            return HTPC_KEY_VK_UP;
        case MELE_KEY_DOWN:
            return HTPC_KEY_VK_DOWN;
        case MELE_KEY_LEFT:
            return HTPC_KEY_VK_LEFT;
        case MELE_KEY_RIGHT:
            return HTPC_KEY_VK_RIGHT;
        case MELE_KEY_HOME:
            return LG_KEY_HOME_MENU;
        case MELE_KEY_VOL_UP:
            return LG_KEY_VOLUME_UP;
        case MELE_KEY_VOL_DOWN:
            return LG_KEY_VOLUME_DOWN;
        case MELE_KEY_FORWARD:
            return LG_KEY_SKIP_FORWARD;
        case MELE_KEY_BACK:
            return LG_KEY_SKIP_BACKWARD;
        case MELE_KEY_ESC:
            return LG_KEY_PREVIOUS_KEY_BACK;
        default:
            break;
    }

    return -1;
}

bool isKeyWithOnlyRelease(int key) {
    switch (key) {
        case MELE_KEY_VOL_UP:
        case MELE_KEY_VOL_DOWN:
        case MELE_KEY_FORWARD:
        case MELE_KEY_BACK:
            return true;
        default:
            break;
    }

    return false;
}
