#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "consumer.h"
#include "buffer.h"

extern time_t lastActivityTime;
extern pthread_cond_t notFull;
extern pthread_cond_t notEmpty;
extern Buffer sharedBuffer;
extern pthread_mutex_t mutex;
extern pthread_mutex_t secondMutex;

extern int running;
extern int totalConsumed;
extern int consumerWaitCount;
extern int consumerSleep;
extern int deadlockMode;

void *consumerFunction(void *arg)
{
    int consumerId = *(int *)arg;
    int item;

    while (running)
    {
        if (deadlockMode)
        {
            pthread_mutex_lock(&secondMutex);
            sleep(1);

            if (pthread_mutex_trylock(&mutex) != 0)
            {
                printf("Consumer C%d could not acquire mutex. Resource contention detected.\n", consumerId);

                pthread_mutex_unlock(&secondMutex);

                sleep(1);
                continue;
            }
        }
        else
        {
            pthread_mutex_lock(&mutex);
        }

        while (sharedBuffer.count == 0 && running)
        {
            consumerWaitCount++;
            printf("Consumer C%d waiting. Buffer is empty.\n", consumerId);
            pthread_cond_wait(&notEmpty, &mutex);
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

        removeItem(&sharedBuffer, &item);

        totalConsumed++;
        lastActivityTime = time(NULL);

        printf("Consumer C%d consumed: %d\n", consumerId, item);
        printBuffer(&sharedBuffer);

        pthread_cond_signal(&notFull);

        pthread_mutex_unlock(&mutex);

        if (deadlockMode)
        {
            pthread_mutex_unlock(&secondMutex);
        }

        sleep(consumerSleep);
    }

    return NULL;
}