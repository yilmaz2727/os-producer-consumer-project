#include "buffer.h"
#include <stdio.h>
void initializeBuffer(Buffer* buffer) {

    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
}

int insertItem(Buffer* buffer, int item) {

    if (buffer->count == BUFFER_SIZE) {
        return -1;
    }

    buffer->items[buffer->in] = item;

    buffer->in = (buffer->in + 1) % BUFFER_SIZE;

    buffer->count++;

    return 0;
}

int removeItem(Buffer* buffer, int* item) {

    if (buffer->count == 0) {
        return -1;
    }

    *item = buffer->items[buffer->out];

    buffer->out = (buffer->out + 1) % BUFFER_SIZE;

    buffer->count--;

    return 0;
}

void printBuffer(Buffer* buffer) {
    printf("Buffer: [");

    for (int i = 0; i < buffer->count; i++) {
        int index = (buffer->out + i) % BUFFER_SIZE;
        printf("%d", buffer->items[index]);

        if (i < buffer->count - 1) {
            printf(" ");
        }
    }

    printf("]\n");
}