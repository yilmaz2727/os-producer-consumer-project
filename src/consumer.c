#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "consumer.h"
#include "buffer.h"
#include "config.h"

extern time_t lastActivityTime;
extern Buffer bufferA;
extern Buffer bufferB;
extern int running;
extern int totalConsumed;
extern int consumerWaitCount;
extern int totalProduced;
extern int deadlockMode; // Bunu geri getirdik

void *consumerFunction(void *arg)
{
    ThreadConfig *tConfig = (ThreadConfig *)arg;
    int item;

    Buffer *inBuf = (tConfig->inBuffer == 'A') ? &bufferA : &bufferB;
    Buffer *outBuf = NULL;
    if(tConfig->outBuffer == 'A') outBuf = &bufferA;
    else if (tConfig->outBuffer == 'B') outBuf = &bufferB;

    while (running)
    {
        // --- KASTİ DEADLOCK SİMÜLASYONU ---
        if (deadlockMode) {
            pthread_mutex_lock(&bufferB.mutex);
            printf("Consumer C%d locked Buffer B (Deadlock test)\n", tConfig->id);
            sleep(1);
            printf("Consumer C%d trying to lock Buffer A...\n", tConfig->id);
            pthread_mutex_lock(&bufferA.mutex);
            
            pthread_mutex_unlock(&bufferA.mutex);
            pthread_mutex_unlock(&bufferB.mutex);
            sleep(1);
            continue;
        }
        // -----------------------------------

        printf("Consumer C%d is trying to Lock Buffer %c...\n", tConfig->id, inBuf->name);
        pthread_mutex_lock(&inBuf->mutex);
        printf("Consumer C%d has Lock on Buffer %c.\n", tConfig->id, inBuf->name);

        while (inBuf->count == 0 && running)
        {
            consumerWaitCount++;
            pthread_cond_wait(&inBuf->notEmpty, &inBuf->mutex);
        }

        if (!running) {
            pthread_mutex_unlock(&inBuf->mutex);
            break;
        }

        removeItem(inBuf, &item);
        totalConsumed++;
        lastActivityTime = time(NULL);
        
        printf("Consumer C%d consumed: %d from Buffer %c\n", tConfig->id, item, inBuf->name);
        printBuffer(inBuf);

        pthread_cond_signal(&inBuf->notFull);
        pthread_mutex_unlock(&inBuf->mutex);
        printf("Consumer C%d released Lock on Buffer %c.\n", tConfig->id, inBuf->name);

        usleep(tConfig->sleepTime * 1000); 

        if (outBuf != NULL && running) {
            printf("Consumer C%d is trying to Lock Buffer %c (to produce)...\n", tConfig->id, outBuf->name);
            pthread_mutex_lock(&outBuf->mutex);
            printf("Consumer C%d has Lock on Buffer %c.\n", tConfig->id, outBuf->name);

            while (outBuf->count == outBuf->capacity && running) {
                pthread_cond_wait(&outBuf->notFull, &outBuf->mutex);
            }

            if(running) {
                insertItem(outBuf, item + 100);
                totalProduced++; 
                printf("Consumer C%d generated and pushed %d to Buffer %c\n", tConfig->id, item+100, outBuf->name);
                printBuffer(outBuf);
                pthread_cond_signal(&outBuf->notEmpty);
            }

            pthread_mutex_unlock(&outBuf->mutex);
            printf("Consumer C%d released Lock on Buffer %c.\n", tConfig->id, outBuf->name);
        }
    }
    return NULL;
}