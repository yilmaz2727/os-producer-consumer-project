#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>

void initializeBuffer(Buffer* buffer, char name, int capacity) {
    buffer->name = name;
    buffer->capacity = capacity;
    buffer->items = (int*)malloc(sizeof(int) * capacity);
    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->notFull, NULL);
    pthread_cond_init(&buffer->notEmpty, NULL);
}

int insertItem(Buffer* buffer, int item) {
    if (buffer->count == buffer->capacity) {
        return -1;
    }
    buffer->items[buffer->in] = item;
    buffer->in = (buffer->in + 1) % buffer->capacity;
    buffer->count++;
    return 0;
}

int removeItem(Buffer* buffer, int* item) {
    if (buffer->count == 0) {
        return -1;
    }
    *item = buffer->items[buffer->out];
    buffer->out = (buffer->out + 1) % buffer->capacity;
    buffer->count--;
    return 0;
}

void printBuffer(Buffer* buffer) {
    printf("Buffer %c: [", buffer->name);
    for (int i = 0; i < buffer->count; i++) {
        int index = (buffer->out + i) % buffer->capacity;
        printf("%d", buffer->items[index]);
        if (i < buffer->count - 1) {
            printf(" ");
        }
    }
    printf("]\n");
}

void destroyBuffer(Buffer* buffer) {
    free(buffer->items);
    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->notFull);
    pthread_cond_destroy(&buffer->notEmpty);
}