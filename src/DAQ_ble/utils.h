#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <WiFi.h>

#define DEBUG_MODE true

#if DEBUG_MODE
#define DEBUG_PRINT(x)	 Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

String getDeviceId();

#endif // UTILS_H
