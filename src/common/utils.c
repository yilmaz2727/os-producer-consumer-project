#include "utils.h"
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>

pthread_mutex_t track_mutex = PTHREAD_MUTEX_INITIALIZER;

// Hangi kaynağı (A veya B) kim tutuyor?
char holder_A_type = 0; int holder_A_id = 0;
char holder_B_type = 0; int holder_B_id = 0;

// Hangi thread hangi kaynağı bekliyor?
char p_waiting[50] = {0}; 
char c_waiting[50] = {0};

void track_waiting(char t_type, int t_id, char resource) {
    pthread_mutex_lock(&track_mutex);
    if (t_type == 'P') p_waiting[t_id] = resource;
    else if (t_type == 'C') c_waiting[t_id] = resource;
    pthread_mutex_unlock(&track_mutex);
}

void track_acquired(char t_type, int t_id, char resource) {
    pthread_mutex_lock(&track_mutex);
    if (t_type == 'P') p_waiting[t_id] = 0;
    else if (t_type == 'C') c_waiting[t_id] = 0;

    if (resource == 'A') { holder_A_type = t_type; holder_A_id = t_id; }
    else if (resource == 'B') { holder_B_type = t_type; holder_B_id = t_id; }
    pthread_mutex_unlock(&track_mutex);
}

void track_released(char t_type, int t_id, char resource) {
    pthread_mutex_lock(&track_mutex);
    if (resource == 'A') { holder_A_type = 0; holder_A_id = 0; }
    else if (resource == 'B') { holder_B_type = 0; holder_B_id = 0; }
    pthread_mutex_unlock(&track_mutex);
}

// Gerçek Deadlock Tespiti (A'yı tutan B'yi bekliyor, B'yi tutan A'yı bekliyorsa = DEADLOCK)
int check_deadlock(void) {
    pthread_mutex_lock(&track_mutex);
    int deadlock = 0;
    if (holder_A_type != 0 && holder_B_type != 0) {
        char A_holder_waiting_for = (holder_A_type == 'P') ? p_waiting[holder_A_id] : c_waiting[holder_A_id];
        char B_holder_waiting_for = (holder_B_type == 'P') ? p_waiting[holder_B_id] : c_waiting[holder_B_id];

        if (A_holder_waiting_for == 'B' && B_holder_waiting_for == 'A') {
            deadlock = 1;
        }
    }
    pthread_mutex_unlock(&track_mutex);
    return deadlock;
}

long long get_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec)*1000) + (tv.tv_usec/1000);
}

void log_event(const char* thread_type, int id, const char* action, char resource) {
    printf("[%lld] %s%d %s Buffer %c\n", get_current_time_ms(), thread_type, id, action, resource);
}