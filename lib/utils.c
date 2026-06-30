#include "../include/minios.h"

char* get_time_str() {
    static char buf[32];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    strftime(buf, 32, "%H:%M:%S", tm);
    return buf;
}

char* get_date_str() {
    static char buf[32];
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    strftime(buf, 32, "%Y-%m-%d", tm);
    return buf;
}

void log_event(const char* msg) {
    printf("[%s] %s\n", get_time_str(), msg);
}
