#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "buffer.h"
#include "producer.h"
#include "consumer.h"
#include "config.h"

Buffer sharedBuffer;
pthread_mutex_t secondMutex;
pthread_mutex_t mutex;
pthread_cond_t notFull;
pthread_cond_t notEmpty;
time_t lastActivityTime;
int deadlockDetected = 0;
int running = 1;

int producerSleep = 2;
int consumerSleep = 1;
int deadlockMode=0;
int totalProduced = 0;
int totalConsumed = 0;
int producerWaitCount = 0;
int consumerWaitCount = 0;
void* deadlockMonitor(void* arg) {

    (void)arg;

    while (running) {
        sleep(1);

        time_t currentTime = time(NULL);

        if (difftime(currentTime, lastActivityTime) >= 5) {
            deadlockDetected = 1;

            printf("\n===== DEADLOCK DETECTED =====\n");
            printf("No producer/consumer activity for 5 seconds.\n");
            printf("Possible circular wait or blocked threads detected.\n");
            printf("=============================\n");

            exit(1);
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
    

    producerSleep = config.producerSleep;
    consumerSleep = config.consumerSleep;
    deadlockMode=config.deadlockMode;

    pthread_t* producers = malloc(sizeof(pthread_t) * config.producerCount);
    pthread_t* consumers = malloc(sizeof(pthread_t) * config.consumerCount);
pthread_t monitorThread;
    int* producerIds = malloc(sizeof(int) * config.producerCount);
    int* consumerIds = malloc(sizeof(int) * config.consumerCount);

    if (producers == NULL || consumers == NULL || producerIds == NULL || consumerIds == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    initializeBuffer(&sharedBuffer);

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&secondMutex, NULL);
    pthread_cond_init(&notFull, NULL);
    pthread_cond_init(&notEmpty, NULL);

    printf("System started with %d producer(s), %d consumer(s), runtime %d second(s).\n",
           config.producerCount,
           config.consumerCount,
           config.runtime);
lastActivityTime = time(NULL);
pthread_create(&monitorThread, NULL, deadlockMonitor, NULL);
    for (int i = 0; i < config.producerCount; i++) {
        producerIds[i] = i + 1;
        pthread_create(&producers[i], NULL, producerFunction, &producerIds[i]);
    }

    for (int i = 0; i < config.consumerCount; i++) {
        consumerIds[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumerFunction, &consumerIds[i]);
    }

    sleep(config.runtime);

    pthread_mutex_lock(&mutex);
    running = 0;
    pthread_join(monitorThread, NULL);
    pthread_cond_broadcast(&notFull);
    pthread_cond_broadcast(&notEmpty);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < config.producerCount; i++) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < config.consumerCount; i++) {
        pthread_join(consumers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&notFull);
    pthread_cond_destroy(&notEmpty);
pthread_mutex_destroy(&secondMutex);
    free(producers);
    free(consumers);
    free(producerIds);
    free(consumerIds);

    printf("System finished.\n");
printf("\n===== PERFORMANCE METRICS =====\n");
printf("Total produced: %d\n", totalProduced);
printf("Total consumed: %d\n", totalConsumed);
printf("Producer wait count: %d\n", producerWaitCount);
printf("Consumer wait count: %d\n", consumerWaitCount);
printf("Throughput: %.2f item/sec\n", (double)totalConsumed / config.runtime);
printf("================================\n");
    return 0;
}
