#ifndef __DEBUG_H
#define __DEBUG_H

#undef NDEBUG
// #define NDEBUG

#ifndef NDEBUG
#define DEBUG_MESSAGE(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_MESSAGE(...) 
#endif

#endif