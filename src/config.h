#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int producerCount;
    int consumerCount;
    int runtime;
    int producerSleep;
    int consumerSleep;
} Config;

void loadConfig(const char* filename, Config* config);

#endif