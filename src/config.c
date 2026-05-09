#include <stdio.h>
#include <string.h>
#include "config.h"

void loadConfig(const char* filename, Config* config) {

    config->producerCount = 2;
    config->consumerCount = 2;
    config->runtime = 60;
    config->producerSleep = 1;
    config->consumerSleep = 1;
    config->deadlockMode = 0;

    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        printf("Config file could not be opened. Using default values.\n");
        return;
    }

    char key[50];
    int value;

    while (fscanf(file, " %49[^=]=%d", key, &value) == 2) {

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
        } else if (strcmp(key, "deadlock_mode") == 0) {
            config->deadlockMode = value;
        }
    }

    fclose(file);

    printf("Configuration loaded:\n");
    printf("Producers: %d\n", config->producerCount);
    printf("Consumers: %d\n", config->consumerCount);
    printf("Runtime: %d seconds\n", config->runtime);
    printf("Producer sleep: %d seconds\n", config->producerSleep);
    printf("Consumer sleep: %d seconds\n", config->consumerSleep);
    printf("Deadlock mode: %d\n", config->deadlockMode);
}