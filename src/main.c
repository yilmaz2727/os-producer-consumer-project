#include <stdio.h>
#include <pthread.h>
#include "buffer.h"
#include "producer.h"
#include "consumer.h"
#include <unistd.h> // sleep(10) için kulladık
#define PRODUCER_COUNT 2
#define CONSUMER_COUNT 2

Buffer sharedBuffer;
pthread_mutex_t mutex;
int running = 1;// 60 snlik süre için

int main() {

    pthread_t producers[PRODUCER_COUNT];
    pthread_t consumers[CONSUMER_COUNT];

    int producerIds[PRODUCER_COUNT];
    int consumerIds[CONSUMER_COUNT];

    initializeBuffer(&sharedBuffer);
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < PRODUCER_COUNT; i++) {
        producerIds[i] = i + 1;
        pthread_create(&producers[i], NULL, producerFunction, &producerIds[i]);
    }

    for (int i = 0; i < CONSUMER_COUNT; i++) {
        consumerIds[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumerFunction, &consumerIds[i]);
    }
    
sleep(10);
running = 0;
    for (int i = 0; i < PRODUCER_COUNT; i++) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < CONSUMER_COUNT; i++) {
        pthread_join(consumers[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    return 0;
}