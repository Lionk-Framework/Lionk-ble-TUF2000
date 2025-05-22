#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <ArduinoBLE.h>
#include "sensor.h"

void bleSetup();
void startAdvertising();
void onFlowHistorySubscribe(BLEDevice central, BLECharacteristic characteristic);
void onFlowHistoryUnsubscribe(BLEDevice central, BLECharacteristic characteristic);
void onPipeDiameterWritten(BLEDevice central, BLECharacteristic characteristic);
bool bleSendFlowHistoryBuffer(const uint16_t *buffer, int length);
bool bleSendVelocityHistoryBuffer(const uint16_t *buffer, int length);
bool bleSendYearlyFlow();
bool bleSendPipeDiameter();

// Subscription flags for each service
extern bool flowHistorySubscribed;
extern bool velocityHistorySubscribed;
extern bool yearlyFlowSubscribed;
extern bool pipeDiameterSubscribed;

// BLE Services and Characteristics
// Pipe diameter service and characteristic (read/write)
extern BLEService pipeDiameterService;
extern BLEUnsignedIntCharacteristic pipeDiameterChar;

// Yearly flow service and characteristic
extern BLEService yearlyFlowService;
extern BLECharacteristic yearlyFlowChar;

// Version information
extern BLEService versionService;
extern BLEStringCharacteristic versionChar;

#endif // BLE_MANAGER_H
