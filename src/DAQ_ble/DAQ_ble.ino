#include "ble_manager.h"
#include "sensor.h"
#include "utils.h"

unsigned long lastUpdateTime = 0;
const long updateInterval = 10;

void setup() {
    if (DEBUG_MODE) {
        Serial.begin(9600);
    }
    pinMode(A0, INPUT); // TODO: Manage correctly
    sensor_data.flow_rate = 0;
    bleSetup();
    DEBUG_PRINTLN("Setup complete");
}

void loop() {
    unsigned long currentTime = millis();
    BLEDevice central = BLE.central();
    if (central) {
        DEBUG_PRINT("Connected to central: ");
        DEBUG_PRINTLN(central.address());
        while (central.connected()) {
            currentTime = millis();
            if (currentTime - lastUpdateTime >= updateInterval) {
                lastUpdateTime = currentTime;
                updateSensorData();
                if (subscribed) {
                    if (bleSendData()) {
                        DEBUG_PRINTLN("Data sent successfully");
                    } else {
                        DEBUG_PRINTLN("Failed to send data");
                    }
                }
            }
            delay(10);
        }
        DEBUG_PRINT("Disconnected from central: ");
        DEBUG_PRINTLN(central.address());
        subscribed = false;
        startAdvertising();
    }
    delay(100); // wait on new connection
}