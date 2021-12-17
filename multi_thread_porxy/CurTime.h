#ifndef SINGLE_THREAD_PORXY_CURTIME_H
#define SINGLE_THREAD_PORXY_CURTIME_H

#include <ctime>

char * getTime() {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime); // текущая дата, выраженная в секундах
    timeinfo = localtime(&rawtime);
    return asctime(timeinfo);
}

#endif //SINGLE_THREAD_PORXY_CURTIME_H
