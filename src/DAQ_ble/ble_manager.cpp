#include "ble_manager.h"
#include "sensor.h"
#include "utils.h"
#include <string.h>

// UUIDs for services
constexpr char UUID_FLOW_SVC[] = "181A";
constexpr char UUID_VELOCITY_SVC[] = "181B";
constexpr char UUID_PIPE_DIAMETER_SVC[] = "181C";
constexpr char UUID_YEARLY_FLOW_SVC[] = "181D";
constexpr char UUID_DATA_SVC[] = "19B10000-E8F2-537E-4F6C-D104768A1214";
constexpr char UUID_VELOCITY_DATA_SVC[] = "19B10010-E8F2-537E-4F6C-D104768A1214";
constexpr char UUID_VERSION_SVC[] = "180A";

// UUIDs for characteristics
constexpr char UUID_FLOW[] = "2A6D";
constexpr char UUID_VELOCITY[] = "2A6E";
constexpr char UUID_PIPE_DIAMETER[] = "2A6F";
constexpr char UUID_YEARLY_FLOW[] = "2A70";
constexpr char UUID_DATA[] = "19B10001-E8F2-537E-4F6C-D104768A1214";
constexpr char UUID_VELOCITY_DATA[] = "19B10011-E8F2-537E-4F6C-D104768A1214";
constexpr char UUID_VERSION[] = "2A28";

char device_name[32];
constexpr const char *VERSION = "1.0.0";

// Subscription flags
bool subscribed = false;
bool velocitySubscribed = false;
bool yearlyFlowSubscribed = false;
bool pipeDiameterSubscribed = false;

// Services and characteristics for flow rate
BLEService flowService(UUID_FLOW_SVC);
BLEUnsignedIntCharacteristic flowChar(UUID_FLOW, BLERead);
BLEService dataService(UUID_DATA_SVC);
BLECharacteristic dataChar(UUID_DATA, BLENotify, PAYLOAD_SIZE);

// Services and characteristics for velocity
BLEService velocityService(UUID_VELOCITY_SVC);
BLEUnsignedIntCharacteristic velocityChar(UUID_VELOCITY, BLERead);
BLEService velocityDataService(UUID_VELOCITY_DATA_SVC);
BLECharacteristic velocityDataChar(UUID_VELOCITY_DATA, BLENotify, PAYLOAD_SIZE);

// Services and characteristics for pipe diameter
BLEService pipeDiameterService(UUID_PIPE_DIAMETER_SVC);
BLEUnsignedIntCharacteristic pipeDiameterChar(UUID_PIPE_DIAMETER, BLERead | BLEWrite);

// Services and characteristics for yearly flow
BLEService yearlyFlowService(UUID_YEARLY_FLOW_SVC);
BLEUnsignedLongCharacteristic yearlyFlowChar(UUID_YEARLY_FLOW, BLERead);

// Service for version
BLEService versionService(UUID_VERSION_SVC);
BLEStringCharacteristic versionChar(UUID_VERSION, BLERead, 20);

void startAdvertising()
{
	BLE.advertise();
	DEBUG_PRINTLN("Started advertising");
}

void bleSetup()
{
        String device_id = getDeviceId();
        DEBUG_PRINTLN(device_id);
        
        snprintf(device_name, sizeof(device_name), "Lionk-Flow-%s", device_id.c_str());
	if (!BLE.begin()) {
		DEBUG_PRINTLN("BLE initialization failed!");
		while (true)
			;
	}
	BLE.setDeviceName(device_name);
	BLE.setLocalName(device_name);
	
	// Configure main service for advertising
	BLE.setAdvertisedService(dataService);
	
	// Configure services and characteristics for flow rate
	flowService.addCharacteristic(flowChar);
	dataService.addCharacteristic(dataChar);
	BLE.addService(flowService);
	BLE.addService(dataService);
	flowChar.writeValue(sensor_data.flow_rate);
	dataChar.setEventHandler(BLESubscribed, onSubscribe);
	dataChar.setEventHandler(BLEUnsubscribed, onUnsubscribe);
	
	// Configure services and characteristics for velocity
	velocityService.addCharacteristic(velocityChar);
	velocityDataService.addCharacteristic(velocityDataChar);
	BLE.addService(velocityService);
	BLE.addService(velocityDataService);
	velocityChar.writeValue(sensor_data.velocity);
	velocityDataChar.setEventHandler(BLESubscribed, 
		[](BLEDevice central, BLECharacteristic ch) {
			velocitySubscribed = true;
			DEBUG_PRINT("Velocity notifications enabled by central: ");
			DEBUG_PRINTLN(central.address());
		});
	velocityDataChar.setEventHandler(BLEUnsubscribed, 
		[](BLEDevice central, BLECharacteristic ch) {
			velocitySubscribed = false;
			DEBUG_PRINTLN("Velocity notifications disabled");
		});
	
	// Configure services and characteristics for pipe diameter
	pipeDiameterService.addCharacteristic(pipeDiameterChar);
	BLE.addService(pipeDiameterService);
	pipeDiameterChar.writeValue(sensor_data.pipe_diameter);
	pipeDiameterChar.setEventHandler(BLESubscribed, 
		[](BLEDevice central, BLECharacteristic ch) {
			pipeDiameterSubscribed = true;
			DEBUG_PRINT("Pipe diameter notifications enabled by central: ");
			DEBUG_PRINTLN(central.address());
		});
	pipeDiameterChar.setEventHandler(BLEUnsubscribed, 
		[](BLEDevice central, BLECharacteristic ch) {
			pipeDiameterSubscribed = false;
			DEBUG_PRINTLN("Pipe diameter notifications disabled");
		});
	pipeDiameterChar.setEventHandler(BLEWritten, onPipeDiameterWritten);
	
	// Configure services and characteristics for yearly flow
	yearlyFlowService.addCharacteristic(yearlyFlowChar);
	BLE.addService(yearlyFlowService);
	yearlyFlowChar.writeValue(sensor_data.yearly_flow);
	yearlyFlowChar.setEventHandler(BLESubscribed, 
		[](BLEDevice central, BLECharacteristic ch) {
			yearlyFlowSubscribed = true;
			DEBUG_PRINT("Yearly flow notifications enabled by central: ");
			DEBUG_PRINTLN(central.address());
		});
	yearlyFlowChar.setEventHandler(BLEUnsubscribed, 
		[](BLEDevice central, BLECharacteristic ch) {
			yearlyFlowSubscribed = false;
			DEBUG_PRINTLN("Yearly flow notifications disabled");
		});
	
	// Configure version service
	versionService.addCharacteristic(versionChar);
	BLE.addService(versionService);
	versionChar.writeValue(VERSION);
	
	startAdvertising();
	DEBUG_PRINTLN("BLE initialized");
	DEBUG_PRINT("Device ID: ");
	DEBUG_PRINTLN(device_id);
	DEBUG_PRINT("Device Name: ");
	DEBUG_PRINTLN(device_name);
}

void onSubscribe(BLEDevice central, BLECharacteristic characteristic)
{
	subscribed = true;
	DEBUG_PRINT("Notifications enabled by central: ");
	DEBUG_PRINTLN(central.address());
}

void onUnsubscribe(BLEDevice central, BLECharacteristic characteristic)
{
	subscribed = false;
	DEBUG_PRINTLN("Notifications disabled");
}

bool bleSendDataBuffer(const uint16_t *samples, int length)
{
	if (!subscribed)
		return false;
	// Build a payload: [version][count][samples...]
	uint8_t payload[PAYLOAD_SIZE];
	payload[0] = PAYLOAD_VERSION;
	payload[1] = length;
	for (int i = 0; i < length; ++i) {
		payload[HEADER_SIZE + i * SAMPLE_SIZE] = (samples[i] >> 8) &
							 0xFF;
		payload[HEADER_SIZE + i * SAMPLE_SIZE + 1] = samples[i] & 0xFF;
	}
	return dataChar.writeValue(payload, HEADER_SIZE + length * SAMPLE_SIZE);
}

bool bleSendVelocityBuffer(const uint16_t *samples, int length)
{
	if (!velocitySubscribed)
		return false;
	
	// Build a payload: [version][count][samples...]
	uint8_t payload[PAYLOAD_SIZE];
	payload[0] = PAYLOAD_VERSION;
	payload[1] = length;
	for (int i = 0; i < length; ++i) {
		payload[HEADER_SIZE + i * SAMPLE_SIZE] = (samples[i] >> 8) &
							 0xFF;
		payload[HEADER_SIZE + i * SAMPLE_SIZE + 1] = samples[i] & 0xFF;
	}
	return velocityDataChar.writeValue(payload, HEADER_SIZE + length * SAMPLE_SIZE);
}

bool bleSendYearlyFlow()
{
	if (!yearlyFlowSubscribed)
		return false;
	
	return yearlyFlowChar.writeValue(sensor_data.yearly_flow);
}

bool bleSendPipeDiameter()
{
	if (!pipeDiameterSubscribed)
		return false;
	
	return pipeDiameterChar.writeValue(sensor_data.pipe_diameter);
}

void onPipeDiameterWritten(BLEDevice central, BLECharacteristic characteristic)
{
	// Read the new value from the characteristic
	uint16_t rawValue = pipeDiameterChar.value();
	
	// Convert from the scaled value (x100) to actual value
	float diameter = static_cast<float>(rawValue) / 100.0f;
	
	DEBUG_PRINT("Pipe diameter written via BLE: ");
	DEBUG_PRINT(diameter);
	DEBUG_PRINTLN(" mm");
	
	// Write the new value to the Modbus device
	if (writeInnerPipeDiameter(diameter)) {
		DEBUG_PRINTLN("Pipe diameter successfully written to Modbus");
		
		// Update our local copy of the value
		sensor_data.pipe_diameter = rawValue;
	} else {
		DEBUG_PRINTLN("Failed to write pipe diameter to Modbus");
		
		// Revert the characteristic to the original value since write failed
		pipeDiameterChar.writeValue(sensor_data.pipe_diameter);
	}
}
