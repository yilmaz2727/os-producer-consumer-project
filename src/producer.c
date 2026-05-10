#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "producer.h"
#include "buffer.h"
#include "config.h"
#include "common/utils.h"

extern time_t lastActivityTime;
extern Buffer bufferA;
extern Buffer bufferB;
extern int running;
extern int totalProduced;
extern int producerWaitCount;
extern int deadlockMode; 

// YENİ: Süre metrikleri için extern tanımlamaları
extern long long totalProducerWaitTimeMs;
extern long long totalProducerBlockTimeMs;

void *producerFunction(void *arg)
{
    ThreadConfig *tConfig = (ThreadConfig *)arg;
    int item = 1;
    Buffer *outBuf = (tConfig->outBuffer == 'A') ? &bufferA : &bufferB;

    while (running)
    {
        if (deadlockMode) {
            track_waiting('P', tConfig->id, 'A');
            pthread_mutex_lock(&bufferA.mutex);
            track_acquired('P', tConfig->id, 'A');
            log_event("Producer", tConfig->id, "locked (Deadlock test)", 'A');
            
            sleep(1);
            
            track_waiting('P', tConfig->id, 'B');
            log_event("Producer", tConfig->id, "is waiting for", 'B');
            pthread_mutex_lock(&bufferB.mutex);
            track_acquired('P', tConfig->id, 'B');
            
            pthread_mutex_unlock(&bufferB.mutex);
            track_released('P', tConfig->id, 'B');
            pthread_mutex_unlock(&bufferA.mutex);
            track_released('P', tConfig->id, 'A');
            sleep(1);
            continue;
        }

        log_event("Producer", tConfig->id, "is trying to lock", outBuf->name);
        track_waiting('P', tConfig->id, outBuf->name);

        // YENİ: Lock alma süresini ölç (Blocking Time)
        long long startLock = get_current_time_ms();
        pthread_mutex_lock(&outBuf->mutex);
        long long endLock = get_current_time_ms();
        __atomic_add_fetch(&totalProducerBlockTimeMs, (endLock - startLock), __ATOMIC_SEQ_CST);

        track_acquired('P', tConfig->id, outBuf->name);
        log_event("Producer", tConfig->id, "has lock on", outBuf->name);

        while (outBuf->count == outBuf->capacity && running)
        {
            __atomic_add_fetch(&producerWaitCount, 1, __ATOMIC_SEQ_CST);
            
            // YENİ: Condition wait (Bekleme) süresini ölç (Waiting Time)
            long long startWait = get_current_time_ms();
            pthread_cond_wait(&outBuf->notFull, &outBuf->mutex);
            long long endWait = get_current_time_ms();
            __atomic_add_fetch(&totalProducerWaitTimeMs, (endWait - startWait), __ATOMIC_SEQ_CST);
        }

        if (!running)
        {
            pthread_mutex_unlock(&outBuf->mutex);
            track_released('P', tConfig->id, outBuf->name);
            break;
        }

        insertItem(outBuf, item);
        __atomic_add_fetch(&totalProduced, 1, __ATOMIC_SEQ_CST); // Thread-safe artırım
        lastActivityTime = time(NULL);

        printf("Producer P%d produced: %d to Buffer %c\n", tConfig->id, item, outBuf->name);
        printBuffer(outBuf);

        item++;

        pthread_cond_signal(&outBuf->notEmpty);
        
        pthread_mutex_unlock(&outBuf->mutex);
        track_released('P', tConfig->id, outBuf->name);
        log_event("Producer", tConfig->id, "released lock on", outBuf->name);

        usleep(tConfig->sleepTime * 1000);
    }
    return NULL;
}