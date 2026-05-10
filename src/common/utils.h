#ifndef UTILS_H
#define UTILS_H

// Deadlock izleme (Wait-For Graph) fonksiyonları
void track_waiting(char thread_type, int thread_id, char resource);
void track_acquired(char thread_type, int thread_id, char resource);
void track_released(char thread_type, int thread_id, char resource);
int check_deadlock(void);

// Performans analizi ve loglama için
long long get_current_time_ms(void);
void log_event(const char* thread_type, int id, const char* action, char resource);

#endif