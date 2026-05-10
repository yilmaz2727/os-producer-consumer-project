#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "buffer.h"
#include "producer.h"
#include "consumer.h"
#include "config.h"
#include "common/utils.h"

// Global Çoklu Buffer Tanımlamaları
Buffer bufferA;
Buffer bufferB;

time_t lastActivityTime;
int deadlockDetected = 0;
int running = 1;
int deadlockMode = 0;

int totalProduced = 0;
int totalConsumed = 0;
int producerWaitCount = 0;
int consumerWaitCount = 0;

void* deadlockMonitor(void* arg) {
    (void)arg;
    while (running) {
        usleep(500000); // Saniyede 2 kez kontrol et
        
        if (check_deadlock()) { // utils.c içerisindeki gerçek döngüsel bekleme (circular wait) kontrolü
            deadlockDetected = 1;

            printf("\n===== CIRCULAR WAIT DEADLOCK DETECTED =====\n");
            printf("Thread A is waiting for Resource B, and Thread B is waiting for Resource A!\n");
            printf("===========================================\n");

            running = 0; // Sistemi durdur
            // Threadleri kilitlerinden kurtarmak için uyandır
            pthread_cond_broadcast(&bufferA.notFull);
            pthread_cond_broadcast(&bufferA.notEmpty);
            pthread_cond_broadcast(&bufferB.notFull);
            pthread_cond_broadcast(&bufferB.notEmpty);
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    Config config;
    const char* configFile = "config.txt";

    if (argc > 1) {
        configFile = argv[1];
    }

    loadConfig(configFile, &config);
    deadlockMode = config.deadlockMode;

    // A ve B Buffer'larını dinamik boyutlarla başlat
    initializeBuffer(&bufferA, 'A', config.bufferASize > 0 ? config.bufferASize : 5);
    initializeBuffer(&bufferB, 'B', config.bufferBSize > 0 ? config.bufferBSize : 5);

    pthread_t* producers = malloc(sizeof(pthread_t) * config.producerCount);
    pthread_t* consumers = malloc(sizeof(pthread_t) * config.consumerCount);
    pthread_t monitorThread;

    lastActivityTime = time(NULL);

    pthread_create(&monitorThread, NULL, deadlockMonitor, NULL);

    for (int i = 0; i < config.producerCount; i++) {
        pthread_create(&producers[i], NULL, producerFunction, &config.producers[i]);
    }

    for (int i = 0; i < config.consumerCount; i++) {
        pthread_create(&consumers[i], NULL, consumerFunction, &config.consumers[i]);
    }

    // Çalışma süresi kadar ana thread'i beklet
    sleep(config.runtime);

    running = 0; // Sistemi durdur
    
    // Tüm bekleyen thread'leri broadcast ile uyandırıp kilitlerinden düşür
    pthread_cond_broadcast(&bufferA.notFull);
    pthread_cond_broadcast(&bufferA.notEmpty);
    pthread_cond_broadcast(&bufferB.notFull);
    pthread_cond_broadcast(&bufferB.notEmpty);

    pthread_join(monitorThread, NULL);

    for (int i = 0; i < config.producerCount; i++) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < config.consumerCount; i++) {
        pthread_join(consumers[i], NULL);
    }

    printf("System finished.\n");
    printf("\n===== PERFORMANCE METRICS =====\n");
    printf("Total produced: %d\n", totalProduced);
    printf("Total consumed: %d\n", totalConsumed);
    printf("Producer wait count: %d\n", producerWaitCount);
    printf("Consumer wait count: %d\n", consumerWaitCount);
    printf("Throughput: %.2f item/sec\n", (double)totalConsumed / config.runtime);
    printf("Deadlock detected: %s\n", deadlockDetected ? "YES" : "NO");
    printf("================================\n");

    destroyBuffer(&bufferA);
    destroyBuffer(&bufferB);
    free(producers);
    free(consumers);

    return 0;
}