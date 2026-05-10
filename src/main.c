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

// YENİ: Süreleri tutacak global değişkenler (Milisaniye cinsinden)
long long totalProducerWaitTimeMs = 0;
long long totalConsumerWaitTimeMs = 0;
long long totalProducerBlockTimeMs = 0;
long long totalConsumerBlockTimeMs = 0;

void* deadlockMonitor(void* arg) {
    (void)arg;
    while (running) {
        usleep(500000); // Saniyede 2 kez kontrol et
        
        if (check_deadlock()) { 
            deadlockDetected = 1;

            printf("\n===== CIRCULAR WAIT DEADLOCK DETECTED =====\n");
            printf("Thread A is waiting for Resource B, and Thread B is waiting for Resource A!\n");
            printf("===========================================\n");

            running = 0; // Sistemi durdur
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

    sleep(config.runtime);

    running = 0; 
    
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

    // Ortalamaları hesapla
    double avgProducerWaitTime = producerWaitCount > 0 ? (double)totalProducerWaitTimeMs / producerWaitCount : 0.0;
    double avgConsumerWaitTime = consumerWaitCount > 0 ? (double)totalConsumerWaitTimeMs / consumerWaitCount : 0.0;

    printf("System finished.\n");
    printf("\n===== PERFORMANCE METRICS =====\n");
    printf("Total produced: %d\n", totalProduced);
    printf("Total consumed: %d\n", totalConsumed);
    printf("Producer wait count: %d\n", producerWaitCount);
    printf("Consumer wait count: %d\n", consumerWaitCount);
    printf("Throughput: %.2f item/sec\n", (double)totalConsumed / config.runtime);
    
    // YENİ: Ortalama Bekleme Süreleri (Average Waiting Time)
    printf("Avg. Producer Wait Time: %.2f ms\n", avgProducerWaitTime);
    printf("Avg. Consumer Wait Time: %.2f ms\n", avgConsumerWaitTime);
    
    // YENİ: İş Parçacığı Bloklanma Süreleri (Thread Blocking Time)
    printf("Total Producer Block Time: %lld ms\n", totalProducerBlockTimeMs);
    printf("Total Consumer Block Time: %lld ms\n", totalConsumerBlockTimeMs);
    
    printf("Deadlock detected: %s\n", deadlockDetected ? "YES" : "NO");
    printf("================================\n");

    destroyBuffer(&bufferA);
    destroyBuffer(&bufferB);
    free(producers);
    free(consumers);

    return 0;
}