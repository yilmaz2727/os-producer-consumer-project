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
extern volatile int running;
extern int totalConsumed;
extern int consumerWaitCount;
extern int totalProduced;
extern int deadlockMode;

// YENİ: Süre metrikleri için extern tanımlamaları
extern long long totalConsumerWaitTimeMs;
extern long long totalConsumerBlockTimeMs;

void *consumerFunction(void *arg)
{
    ThreadConfig *tConfig = (ThreadConfig *)arg;
    int item;

    Buffer *inBuf = (tConfig->inBuffer == 'A') ? &bufferA : &bufferB;
    Buffer *outBuf = NULL;
    if (tConfig->outBuffer == 'A')
        outBuf = &bufferA;
    else if (tConfig->outBuffer == 'B')
        outBuf = &bufferB;

    while (running)
    {
        if (deadlockMode)
        {
            track_waiting('C', tConfig->id, 'B');

            if (pthread_mutex_trylock(&bufferB.mutex) != 0)
            {
                usleep(100000);
                continue;
            }

            track_acquired('C', tConfig->id, 'B');
            log_event("Consumer", tConfig->id, "locked (Deadlock test)", 'B');

            sleep(1);

            track_waiting('C', tConfig->id, 'A');
            log_event("Consumer", tConfig->id, "is waiting for", 'A');

            if (pthread_mutex_trylock(&bufferA.mutex) != 0)
            {
                while (running && pthread_mutex_trylock(&bufferA.mutex) != 0)
                {
                    usleep(100000);
                }

                if (!running)
                {
                    pthread_mutex_unlock(&bufferB.mutex);
                    track_released('C', tConfig->id, 'B');
                    break;
                }
            }

            track_acquired('C', tConfig->id, 'A');

            pthread_mutex_unlock(&bufferA.mutex);
            track_released('C', tConfig->id, 'A');

            pthread_mutex_unlock(&bufferB.mutex);
            track_released('C', tConfig->id, 'B');

            sleep(1);
            continue;
        }

        log_event("Consumer", tConfig->id, "is trying to lock", inBuf->name);
        track_waiting('C', tConfig->id, inBuf->name);

        // YENİ: Lock alma süresini ölç
        long long startLock = get_current_time_ms();
        pthread_mutex_lock(&inBuf->mutex);
        long long endLock = get_current_time_ms();
        __atomic_add_fetch(&totalConsumerBlockTimeMs, (endLock - startLock), __ATOMIC_SEQ_CST);

        track_acquired('C', tConfig->id, inBuf->name);
        log_event("Consumer", tConfig->id, "has lock on", inBuf->name);

        while (inBuf->count == 0 && running)
        {
            __atomic_add_fetch(&consumerWaitCount, 1, __ATOMIC_SEQ_CST);

            // YENİ: Condition wait süresini ölç
            long long startWait = get_current_time_ms();
            pthread_cond_wait(&inBuf->notEmpty, &inBuf->mutex);
            long long endWait = get_current_time_ms();
            __atomic_add_fetch(&totalConsumerWaitTimeMs, (endWait - startWait), __ATOMIC_SEQ_CST);
        }

        if (!running)
        {
            pthread_mutex_unlock(&inBuf->mutex);
            track_released('C', tConfig->id, inBuf->name);
            break;
        }

        removeItem(inBuf, &item);
        __atomic_add_fetch(&totalConsumed, 1, __ATOMIC_SEQ_CST); // Thread-safe artırım
        lastActivityTime = time(NULL);

        printf("Consumer C%d consumed: %d from Buffer %c\n", tConfig->id, item, inBuf->name);
        printBuffer(inBuf);

        pthread_cond_signal(&inBuf->notFull);

        pthread_mutex_unlock(&inBuf->mutex);
        track_released('C', tConfig->id, inBuf->name);
        log_event("Consumer", tConfig->id, "released lock on", inBuf->name);

        usleep(tConfig->sleepTime * 1000);

        // Eğer bu tüketici aynı zamanda yeni bir veri üretecekse (Circular dependency)
        if (outBuf != NULL && running)
        {
            log_event("Consumer", tConfig->id, "is trying to lock (to produce)", outBuf->name);
            track_waiting('C', tConfig->id, outBuf->name);

            // YENİ: İkinci Buffer için lock süresini ölç
            long long startOutLock = get_current_time_ms();
            pthread_mutex_lock(&outBuf->mutex);
            long long endOutLock = get_current_time_ms();
            __atomic_add_fetch(&totalConsumerBlockTimeMs, (endOutLock - startOutLock), __ATOMIC_SEQ_CST);

            track_acquired('C', tConfig->id, outBuf->name);
            log_event("Consumer", tConfig->id, "has lock on", outBuf->name);

            while (outBuf->count == outBuf->capacity && running)
            {
                // YENİ: İkinci Buffer için wait süresini ölç
                long long startOutWait = get_current_time_ms();
                pthread_cond_wait(&outBuf->notFull, &outBuf->mutex);
                long long endOutWait = get_current_time_ms();
                __atomic_add_fetch(&totalConsumerWaitTimeMs, (endOutWait - startOutWait), __ATOMIC_SEQ_CST);
            }

            if (running)
            {
                insertItem(outBuf, item + 100);
                __atomic_add_fetch(&totalProduced, 1, __ATOMIC_SEQ_CST);
                printf("Consumer C%d generated and pushed %d to Buffer %c\n", tConfig->id, item + 100, outBuf->name);
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