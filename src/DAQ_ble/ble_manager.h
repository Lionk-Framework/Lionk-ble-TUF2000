#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <ArduinoBLE.h>

#include "sensor.h"

void bleSetup();
void startAdvertising();
void onSubscribe(BLEDevice central, BLECharacteristic characteristic);
void onUnsubscribe(BLEDevice central, BLECharacteristic characteristic);
bool bleSendData();
extern bool subscribed;
extern BLEService flowService;
extern BLEUnsignedIntCharacteristic flowChar;
extern BLEService dataService;
extern BLECharacteristic dataChar;
extern BLEService versionService;
extern BLEStringCharacteristic versionChar;

#endif // BLE_MANAGER_H
