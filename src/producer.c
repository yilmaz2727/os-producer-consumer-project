#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "producer.h"
#include "buffer.h"
#include <pthread.h>
extern Buffer sharedBuffer;
extern pthread_mutex_t mutex;
extern int running; // from main
void* producerFunction(void* arg) {
 int producerId = *(int*)arg;
    int item = 1;

    while (running) {
        pthread_mutex_lock(&mutex);

        if (insertItem(&sharedBuffer, item) == 0) {
          printf("Producer P%d produced: %d\n", producerId, item);
          printBuffer(&sharedBuffer);
            item++;
        } else {
            printf("Producer P%d waiting. Buffer is full.\n", producerId);
        }
        pthread_mutex_unlock(&mutex);

        sleep(1);
    }

    return NULL;
}