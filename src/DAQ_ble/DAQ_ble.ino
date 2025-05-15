#include "ble_manager.h"
#include "sensor.h"
#include "utils.h"

unsigned long lastUpdateTime = 0;
const long sampleInterval = 10; // 10ms per sample

uint16_t flowRateBuffer[SAMPLES_PER_PACKET];
int bufferIndex = 0;

unsigned long lastSendTime = 0;
const long sendInterval = sampleInterval * SAMPLES_PER_PACKET; // 100ms

void setup()
{
	if (DEBUG_MODE) {
		Serial.begin(9600);
	}
	pinMode(A0, INPUT); // TODO: Manage correctly
	sensor_data.flow_rate = 0;
	bleSetup();
	DEBUG_PRINTLN("Setup complete");
}

void handleCentralConnection(BLEDevice &central)
{
	DEBUG_PRINT("Connected to central: ");
	DEBUG_PRINTLN(central.address());
	while (central.connected()) {
		handleSensorUpdate();
		delay(10);
	}
	DEBUG_PRINT("Disconnected from central: ");
	DEBUG_PRINTLN(central.address());
	subscribed = false;
	startAdvertising();
}

void handleSensorUpdate()
{
	unsigned long currentTime = millis();
	if (currentTime - lastUpdateTime >= sampleInterval) {
		lastUpdateTime = currentTime;
		updateSensorData();
		flowRateBuffer[bufferIndex] = sensor_data.flow_rate;
		bufferIndex++;
		if (bufferIndex >= SAMPLES_PER_PACKET) {
			if (subscribed) {
				if (bleSendDataBuffer(flowRateBuffer,
						      SAMPLES_PER_PACKET)) {
					DEBUG_PRINTLN(
						"Data buffer sent successfully");
				} else {
					DEBUG_PRINTLN(
						"Failed to send data buffer");
				}
			}
			bufferIndex = 0;
		}
	}
}

void loop()
{
	BLEDevice central = BLE.central();
	if (central) {
		handleCentralConnection(central);
	}
	delay(100); // wait on new connection
}
