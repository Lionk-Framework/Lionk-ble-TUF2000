#include "ble_manager.h"
#include "utils.h"

// Last update timestamp for each data type
unsigned long lastFlowVelocityUpdateTime = 0;
unsigned long lastYearlyFlowUpdateTime = 0;

// Sampling interval for Flow and Velocity (20ms to provide 50 samples/second)
const long sampleInterval = FLOW_VELOCITY_UPDATE_INTERVAL;

// Data buffers
uint16_t flowRateBuffer[SAMPLES_PER_PACKET];
uint16_t velocityBuffer[SAMPLES_PER_PACKET];
int flowBufferIndex = 0;
int velocityBufferIndex = 0;

void setup()
{
	if (DEBUG_MODE) {
		Serial.begin(9600);
	}
	sensor_data.flow_rate = 0;
	bleSetup();
	modbusSetup();
	DEBUG_PRINTLN("Setup complete");
}

void handleCentralConnection(BLEDevice &central)
{
	DEBUG_PRINT("Connected to central: ");
	DEBUG_PRINTLN(central.address());
	while (central.connected()) {
        if (yearlyFlowSubscribed ||
            pipeDiameterSubscribed ||
            flowHistorySubscribed ||
            velocityHistorySubscribed) {
                DEBUG_PRINT("Subscribed baby");
                handleSensorUpdate();
        }
		delay(sampleInterval / 2);
	}
	DEBUG_PRINT("Disconnected from central: ");
	DEBUG_PRINTLN(central.address());
	flowHistorySubscribed = false;
	velocityHistorySubscribed = false;
	yearlyFlowSubscribed = false;
	pipeDiameterSubscribed = false;
	startAdvertising();
}

void handleSensorUpdate()
{
	unsigned long currentTime = millis();
	
	// Update flow rate and velocity (every 20ms)
	if (currentTime - lastFlowVelocityUpdateTime >= sampleInterval) {
		lastFlowVelocityUpdateTime = currentTime;

		// Read sensor data via Modbus
		updateSensorData();
		
		// Add data to buffers
		flowRateBuffer[flowBufferIndex] = sensor_data.flow_rate;
		velocityBuffer[velocityBufferIndex] = sensor_data.velocity;
		
		flowBufferIndex++;
		velocityBufferIndex++;
		
		// Check if buffers are full
		if (flowBufferIndex >= SAMPLES_PER_PACKET) {
			if (flowHistorySubscribed) {
				if (bleSendFlowHistoryBuffer(flowRateBuffer, SAMPLES_PER_PACKET)) {
					DEBUG_PRINTLN("Flow rate buffer sent successfully");
				} else {
					DEBUG_PRINTLN("Failed to send flow rate buffer");
				}
			}
			flowBufferIndex = 0;
		}
		
		if (velocityBufferIndex >= SAMPLES_PER_PACKET) {
			if (velocityHistorySubscribed) {
				if (bleSendVelocityHistoryBuffer(velocityBuffer, SAMPLES_PER_PACKET)) {
					DEBUG_PRINTLN("Velocity buffer sent successfully");
				} else {
					DEBUG_PRINTLN("Failed to send velocity buffer");
				}
			}
			velocityBufferIndex = 0;
		}
	}
	
	// Update yearly flow (every 10 seconds)
	if (currentTime - lastYearlyFlowUpdateTime >= YEARLY_FLOW_UPDATE_INTERVAL) {
		lastYearlyFlowUpdateTime = currentTime;
		
		// Read yearly flow data via Modbus
		updateYearlyFlowData();
		
		// Send data if necessary
		if (yearlyFlowSubscribed) {
			if (bleSendYearlyFlow()) {
				DEBUG_PRINTLN("Yearly flow data sent successfully");
			} else {
				DEBUG_PRINTLN("Failed to send yearly flow data");
			}
		}
		
		// Update pipe diameter at the same time
		if (pipeDiameterSubscribed) {
			if (bleSendPipeDiameter()) {
				DEBUG_PRINTLN("Pipe diameter data sent successfully");
			} else {
				DEBUG_PRINTLN("Failed to send pipe diameter data");
			}
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
