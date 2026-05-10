#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "consumer.h"
#include "buffer.h"
#include "config.h"
#include "common/utils.h"

extern time_t lastActivityTime;
extern Buffer bufferA;
extern Buffer bufferB;
extern int running;
extern int totalConsumed;
extern int consumerWaitCount;
extern int totalProduced;
extern int deadlockMode; 

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
            track_waiting('C', tConfig->id, 'B');
            pthread_mutex_lock(&bufferB.mutex);
            track_acquired('C', tConfig->id, 'B');
            log_event("Consumer", tConfig->id, "locked (Deadlock test)", 'B');
            
            sleep(1);
            
            track_waiting('C', tConfig->id, 'A');
            log_event("Consumer", tConfig->id, "is waiting for", 'A');
            pthread_mutex_lock(&bufferA.mutex);
            track_acquired('C', tConfig->id, 'A');
            
            pthread_mutex_unlock(&bufferA.mutex);
            track_released('C', tConfig->id, 'A');
            pthread_mutex_unlock(&bufferB.mutex);
            track_released('C', tConfig->id, 'B');
            sleep(1);
            continue;
        }
        // -----------------------------------

        log_event("Consumer", tConfig->id, "is trying to lock", inBuf->name);
        track_waiting('C', tConfig->id, inBuf->name);

        pthread_mutex_lock(&inBuf->mutex);

        track_acquired('C', tConfig->id, inBuf->name);
        log_event("Consumer", tConfig->id, "has lock on", inBuf->name);

        while (inBuf->count == 0 && running)
        {
            consumerWaitCount++;
            pthread_cond_wait(&inBuf->notEmpty, &inBuf->mutex);
        }

        if (!running) {
            pthread_mutex_unlock(&inBuf->mutex);
            track_released('C', tConfig->id, inBuf->name);
            break;
        }

        removeItem(inBuf, &item);
        totalConsumed++;
        lastActivityTime = time(NULL);
        
        printf("Consumer C%d consumed: %d from Buffer %c\n", tConfig->id, item, inBuf->name);
        printBuffer(inBuf);

        pthread_cond_signal(&inBuf->notFull);
        
        pthread_mutex_unlock(&inBuf->mutex);
        track_released('C', tConfig->id, inBuf->name);
        log_event("Consumer", tConfig->id, "released lock on", inBuf->name);

        usleep(tConfig->sleepTime * 1000); 

        // Eğer bu tüketici aynı zamanda yeni bir veri üretecekse (Circular dependency)
        if (outBuf != NULL && running) {
            log_event("Consumer", tConfig->id, "is trying to lock (to produce)", outBuf->name);
            track_waiting('C', tConfig->id, outBuf->name);

            pthread_mutex_lock(&outBuf->mutex);

            track_acquired('C', tConfig->id, outBuf->name);
            log_event("Consumer", tConfig->id, "has lock on", outBuf->name);

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
            track_released('C', tConfig->id, outBuf->name);
            log_event("Consumer", tConfig->id, "released lock on", outBuf->name);
        }
    }
    return NULL;
}