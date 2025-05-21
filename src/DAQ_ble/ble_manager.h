#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <ArduinoBLE.h>
#include "sensor.h"

void bleSetup();
void startAdvertising();
void onSubscribe(BLEDevice central, BLECharacteristic characteristic);
void onUnsubscribe(BLEDevice central, BLECharacteristic characteristic);
void onPipeDiameterWritten(BLEDevice central, BLECharacteristic characteristic);
bool bleSendDataBuffer(const uint16_t *buffer, int length);
bool bleSendVelocityBuffer(const uint16_t *buffer, int length);
bool bleSendYearlyFlow();
bool bleSendPipeDiameter();

// Flags de souscription pour chaque service
extern bool subscribed;
extern bool velocitySubscribed;
extern bool yearlyFlowSubscribed;
extern bool pipeDiameterSubscribed;

// Services et caractéristiques BLE
extern BLEService flowService;
extern BLEUnsignedIntCharacteristic flowChar;
extern BLEService velocityService;
extern BLEUnsignedIntCharacteristic velocityChar;
extern BLEService pipeDiameterService;
extern BLEUnsignedIntCharacteristic pipeDiameterChar;
extern BLEService yearlyFlowService;
extern BLEUnsignedLongCharacteristic yearlyFlowChar;
extern BLEService dataService;
extern BLECharacteristic dataChar;
extern BLEService velocityDataService;
extern BLECharacteristic velocityDataChar;
extern BLEService versionService;
extern BLEStringCharacteristic versionChar;

#endif // BLE_MANAGER_H
