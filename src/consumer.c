#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "consumer.h"
#include "buffer.h"

extern Buffer sharedBuffer;
extern pthread_mutex_t mutex;
extern int running;//form main
void* consumerFunction(void* arg) {
 int consumerId= *(int*)arg;
    int item;

    while (running) {
        pthread_mutex_lock(&mutex);

        if (removeItem(&sharedBuffer, &item) == 0) {
           printf("Consumer C%d consumed: %d\n", consumerId, item);
           printBuffer(&sharedBuffer);
        } else {
           printf("Consumer C%d waiting. Buffer is empty.\n", consumerId);
        }
        pthread_mutex_unlock(&mutex);

        sleep(2);
    }

    return NULL;
}