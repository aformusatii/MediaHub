#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

bool alphanumeric(char c) {
	return c != ' ' && c != '\n' && c != '\r' && c != '\t' && c != '\0';
}

char* substring(const char *value, const char *start, const char *end) {
	int i;
	int len = strlen(value);

	int startCount = 0;
	int startLen = (start == NULL) ? 0 : strlen(start);
	bool startDetected = (start == NULL);
	int startPos = -1;

	int endCount = 0;
	int endLen = (end == NULL) ? 0 : strlen(end);
	bool endDetected = (end == NULL);
	int endPos = -1;

	for (i = 0; i < len; i++) {
		char c = value[i];

		if (startDetected) {

			if ((startPos < 0) && alphanumeric(c)) {
				startPos = i;
			}

			if (!endDetected && c == end[endCount++]) {
				if (endCount == endLen) {
					endDetected = true;
					break;
				}
			} else {
				for (int j = (i - (endCount - (endDetected ? 0 : 1))); j <= i; j++)  {
					if (alphanumeric(value[j])) {
						endPos = j;
					}
				}

				endCount = 0;
			}
		} else {
			if (c == start[startCount++]) {
				if (startCount == startLen) {
					startDetected = true;
				}
			} else {
				startCount = 0;
			}
		}
	}

	if (endDetected && ((endPos - startPos) > -1)) {
		char *subValue = (char *) malloc((endPos - startPos)+2);
		for (i = startPos; i <= endPos; i++) {
			subValue[i - startPos] = value[i];
		}
		subValue[i - startPos] = 0;
		return subValue;
	}

	return NULL;
}

bool checkIfHostIsUp(const char *host, int retries, int timeout) {
    char command[50];
    memset(command, 0, sizeof(command));
    sprintf(command, "ping -W %d -c 1 %s", timeout, host);

    int firstCheck = -1;

    while (retries--) {
        // First check
        if (firstCheck == -1) {
            firstCheck = (system(command) == 0) ? 0 : 1;
            printf("firstCheck: [%d]\n", firstCheck);
        } else {
            int nextCheck = (system(command) == 0) ? 0 : 1;
            printf("nextCheck: [%d]\n", nextCheck);
            // Check if ping result changed then return it
            if (nextCheck == firstCheck) {
                if (nextCheck == 0) {
                    usleep(100000);
                }
                continue;
            }

            return (nextCheck == 0);
        }
    }

    return (firstCheck == 0);
}
