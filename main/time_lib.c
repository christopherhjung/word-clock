#include "time_lib.h"
#include <stdint.h>
#include <sys/time.h>


inline unsigned long micros() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

uint32_t elapsedTime(uint32_t lastTime ){
    uint32_t currentTime = micros();

    if( lastTime > currentTime ){
        return lastTime - currentTime - 0xffffffff;
    }else{
        return currentTime - lastTime;
    }
}