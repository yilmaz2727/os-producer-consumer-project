#include <stdio.h>
#include <string.h>
#include "config.h"

void loadConfig(const char* filename, Config* config) {

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Config file could not be opened. Using default values.\n");

        config->producerCount = 2;
        config->consumerCount = 2;
        config->runtime = 10;
        config->producerSleep = 2;
        config->consumerSleep = 1;

        return;
    }

    char key[50];
    int value;

    while (fscanf(file, "%49[^=]=%d\n", key, &value) == 2) {

        if (strcmp(key, "producers") == 0) {
            config->producerCount = value;
        } else if (strcmp(key, "consumers") == 0) {
            config->consumerCount = value;
        } else if (strcmp(key, "runtime") == 0) {
            config->runtime = value;
        } else if (strcmp(key, "producer_sleep") == 0) {
            config->producerSleep = value;
        } else if (strcmp(key, "consumer_sleep") == 0) {
            config->consumerSleep = value;
        }
    }

    fclose(file);
}