#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "producer.h"
#include "buffer.h"
#include <pthread.h>
#include <time.h>

extern int deadlockMode;
extern Buffer sharedBuffer;
extern pthread_mutex_t mutex;
extern pthread_mutex_t secondMutex;
extern int running;
extern pthread_cond_t notFull;
extern pthread_cond_t notEmpty;
extern int totalProduced;
extern int producerWaitCount;
extern int producerSleep;
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

            if (pthread_mutex_trylock(&secondMutex) != 0)
            {
                printf("Producer P%d could not acquire secondMutex. Resource contention detected.\n", producerId);

                pthread_mutex_unlock(&mutex);

                sleep(1);
                continue;
            }
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

            if (deadlockMode)
            {
                pthread_mutex_unlock(&secondMutex);
            }

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