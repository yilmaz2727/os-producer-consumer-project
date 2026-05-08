#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "consumer.h"
#include "buffer.h"
#include <time.h>
extern time_t lastActivityTime;
extern pthread_cond_t notFull;
extern pthread_cond_t notEmpty;
extern Buffer sharedBuffer;
extern pthread_mutex_t mutex;
extern int running;//form main 
extern int totalConsumed;
extern int consumerWaitCount;
extern int consumerSleep;//from config.h
extern pthread_mutex_t secondMutex;
void* consumerFunction(void* arg) {

    int consumerId = *(int*)arg;
    int item;

    while (running) {
pthread_mutex_lock(&secondMutex);

sleep(1);

pthread_mutex_lock(&mutex);
        while (sharedBuffer.count == 0 && running) {
         consumerWaitCount++;
            printf("Consumer C%d waiting. Buffer is empty.\n", consumerId);
            pthread_cond_wait(&notEmpty, &mutex);
        }

        if (!running) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        removeItem(&sharedBuffer, &item);
totalConsumed++;
lastActivityTime = time(NULL);
        printf("Consumer C%d consumed: %d\n", consumerId, item);
        printBuffer(&sharedBuffer);

        pthread_cond_signal(&notFull);
pthread_mutex_unlock(&secondMutex);
        pthread_mutex_unlock(&mutex);

        sleep(consumerSleep);
    }

    return NULL;
}