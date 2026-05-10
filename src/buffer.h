#ifndef BUFFER_H
#define BUFFER_H
#include <pthread.h>

typedef struct {
    char name;           // 'A' veya 'B' gibi buffer adı
    int *items;          // Dinamik dizi
    int capacity;        // Konfigürasyondan gelecek boyut (örn: A[22])
    int in;
    int out;
    int count;
    pthread_mutex_t mutex;       // Her buffer'ın kendi mutex'i
    pthread_cond_t notFull;      // Her buffer'ın kendi condition variable'ı
    pthread_cond_t notEmpty;
} Buffer;

void initializeBuffer(Buffer* buffer, char name, int capacity);
int insertItem(Buffer* buffer, int item);
int removeItem(Buffer* buffer, int* item);
void printBuffer(Buffer* buffer);
void destroyBuffer(Buffer* buffer);

#endif