#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    int id;
    char inBuffer;   // Hangi buffer'dan okuyacak (örn: 'A', 'B' veya 0)
    char outBuffer;  // Hangi buffer'a yazacak (örn: 'A', 'B' veya 0)
    int sleepTime;   // Milisaniye cinsinden uyku süresi
} ThreadConfig;

typedef struct {
    int bufferASize;
    int bufferBSize;
    int runtime;
    int deadlockMode;
    
    ThreadConfig producers[50];
    int producerCount;
    
    ThreadConfig consumers[50];
    int consumerCount;
} Config;

void loadConfig(const char* filename, Config* config);

#endif