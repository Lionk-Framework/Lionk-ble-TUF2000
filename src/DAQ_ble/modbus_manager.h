#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <stdint.h>
#include <Arduino.h>

void modbusSetup();
bool readFlowValue(float &flow);
bool readVelocityValue(float &velocity);
bool readInnerPipeDiameter(float &diameter);
bool readYearlyFlow(uint32_t &yearlyFlow);
bool writeInnerPipeDiameter(float diameter);

#endif // MODBUS_MANAGER_H
