#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "producer.h"
#include "buffer.h"
#include "config.h"

extern time_t lastActivityTime;
extern Buffer bufferA;
extern Buffer bufferB;
extern int running;
extern int totalProduced;
extern int producerWaitCount;
extern int deadlockMode; // Bunu geri getirdik

void *producerFunction(void *arg)
{
    ThreadConfig *tConfig = (ThreadConfig *)arg;
    int item = 1;
    Buffer *outBuf = (tConfig->outBuffer == 'A') ? &bufferA : &bufferB;

    while (running)
    {
        // --- KASTİ DEADLOCK SİMÜLASYONU ---
        if (deadlockMode) {
            pthread_mutex_lock(&bufferA.mutex);
            printf("Producer P%d locked Buffer A (Deadlock test)\n", tConfig->id);
            sleep(1);
            printf("Producer P%d trying to lock Buffer B...\n", tConfig->id);
            pthread_mutex_lock(&bufferB.mutex);
            
            pthread_mutex_unlock(&bufferB.mutex);
            pthread_mutex_unlock(&bufferA.mutex);
            sleep(1);
            continue;
        }
        // -----------------------------------

        printf("Producer P%d is trying to Lock Buffer %c...\n", tConfig->id, outBuf->name);
        pthread_mutex_lock(&outBuf->mutex);
        printf("Producer P%d has Lock on Buffer %c.\n", tConfig->id, outBuf->name);

        while (outBuf->count == outBuf->capacity && running)
        {
            producerWaitCount++;
            pthread_cond_wait(&outBuf->notFull, &outBuf->mutex);
        }

        if (!running)
        {
            pthread_mutex_unlock(&outBuf->mutex);
            break;
        }

        insertItem(outBuf, item);
        totalProduced++;
        lastActivityTime = time(NULL);

        printf("Producer P%d produced: %d to Buffer %c\n", tConfig->id, item, outBuf->name);
        printBuffer(outBuf);

        item++;

        pthread_cond_signal(&outBuf->notEmpty);
        pthread_mutex_unlock(&outBuf->mutex);
        printf("Producer P%d released Lock on Buffer %c.\n", tConfig->id, outBuf->name);

        usleep(tConfig->sleepTime * 1000);
    }
    return NULL;
}