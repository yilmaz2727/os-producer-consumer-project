#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "producer.h"
#include "buffer.h"
#include <pthread.h>
#include <time.h>
extern int deadlockMode;
extern Buffer sharedBuffer;     // from main
extern pthread_mutex_t mutex;   // from main
extern int running;             // from main
extern pthread_cond_t notFull;  // from main
extern pthread_cond_t notEmpty; // from main
extern int totalProduced;
extern int producerWaitCount;
extern int producerSleep; // from config.h
extern pthread_mutex_t secondMutex;
extern time_t lastActivityTime;
void *producerFunction(void *arg)
{

    int producerId = *(int *)arg;
    int item = 1;

    while (running)
    {
        pthread_mutex_lock(&mutex);

        if (deadlockMode)
        {
            sleep(1);
            pthread_mutex_lock(&secondMutex);
        }
        while (sharedBuffer.count == BUFFER_SIZE && running)
        {
            producerWaitCount++;
            printf("Producer P%d waiting. Buffer is full.\n", producerId);

            pthread_cond_wait(&notFull, &mutex);
        }

        if (!running)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        insertItem(&sharedBuffer, item);
        totalProduced++;
        lastActivityTime = time(NULL);
        printf("Producer P%d produced: %d\n", producerId, item);
        printBuffer(&sharedBuffer);

        item++;

        pthread_cond_signal(&notEmpty);
        if (deadlockMode)
        {
            pthread_mutex_unlock(&secondMutex);
        }

        pthread_mutex_unlock(&mutex);

        sleep(producerSleep);
    }

    return NULL;
}