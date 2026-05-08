#ifndef BUFFER_H
#define BUFFER_H
#include <pthread.h>
#define BUFFER_SIZE 5

typedef struct {

    int items[BUFFER_SIZE];// buffer dizisi
    int in;// ekleme indexi
    int out;// çıkarma indexi
    int count;// kaç eleman var 

} Buffer;

void initializeBuffer(Buffer* buffer);

int insertItem(Buffer* buffer, int item);

int removeItem(Buffer* buffer, int* item);

void printBuffer(Buffer* buffer);
#endif