#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <WiFi.h>

#define DEBUG_MODE false
#define DEBUG_PRINT(x) if(DEBUG_MODE) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DEBUG_MODE) Serial.println(x)
inline void DEBUG_PRINTLN2(unsigned long long x, int y) { if (DEBUG_MODE) Serial.println(x, y); }

uint64_t getDeviceId();

#endif // UTILS_H
