#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <stdint.h>
#include <Arduino.h>

void modbusSetup();
bool readFlowValue(float &flow);

#endif // MODBUS_MANAGER_H
