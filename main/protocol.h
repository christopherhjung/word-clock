//
// Created by Christopher Jung on 23.10.21.
//

#pragma once
#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <cstdlib>

template<class T>
T next(uint8_t*& ptr){
    T result;
    uint8_t *current = (uint8_t*) &result;
    for(uint8_t size = sizeof(T) ;size > 0; size--){
        *current++ = *ptr++;
    }
    return result;
}

template<uint8_t>
uint8_t next(uint8_t*& ptr){
    return *ptr++;
}

String nextString(uint8_t*& ptr);