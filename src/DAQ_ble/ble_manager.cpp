#include "ble_manager.h"
#include "sensor.h"
#include "utils.h"
#include <string.h>

// Custom services using Lionk's base UUID: 8EC9XXXX-F315-4F60-9FB8-838830549FD2
constexpr char UUID_PIPE_DIAMETER_SVC[] = "19B10030-E8F2-537E-4F6C-D104768A1214";  // Pipe diameter service
constexpr char UUID_YEARLY_FLOW_SVC[] = "19B10020-E8F2-537E-4F6C-D104768A1214";    // Yearly flow service
constexpr char UUID_FLOW_SVC[] = "19B10000-F315-4F60-9FB8-838830549FD2";           // Flow history data service
constexpr char UUID_VELOCITY_DATA_SVC[] = "19B10010-E8F2-537E-4F6C-D104768A1214";  // Velocity history data service
constexpr char UUID_VERSION_SVC[] = "19B10040-F315-4F60-9FB8-838830549FD2";  // Standard Device Information Service

// Custom characteristics
constexpr char UUID_PIPE_DIAMETER[] = "19B10031-E8F2-537E-4F6C-D104768A1214";     // Pipe diameter characteristic 
constexpr char UUID_YEARLY_FLOW[] = "19B10021-E8F2-537E-4F6C-D104768A1214";       // Yearly flow characteristic
constexpr char UUID_FLOW_DATA[] = "19B10001-F315-4F60-9FB8-838830549FD2";              // Flow history data characteristic
constexpr char UUID_VELOCITY_DATA[] = "19B10011-E8F2-537E-4F6C-D104768A1214";     // Velocity history data characteristic
constexpr char UUID_VERSION[] = "19B10041-F315-4F60-9FB8-838830549FD2";  // Standard Firmware Revision String

char device_name[32];
constexpr const char *VERSION = "1.0.0";

// Subscription flags
bool flowHistorySubscribed = false;
bool velocityHistorySubscribed = false;
bool yearlyFlowSubscribed = false;
bool pipeDiameterSubscribed = false;

// Flow history service and characteristic
BLEService flowHistoryService(UUID_FLOW_SVC);                          // Flow history data service
BLECharacteristic flowHistoryChar(UUID_FLOW_DATA, BLENotify, PAYLOAD_SIZE); // Flow history data characteristic (last 50 values)

// Velocity history service and characteristic
BLEService velocityHistoryService(UUID_VELOCITY_DATA_SVC);                 // Velocity history data service
BLECharacteristic velocityHistoryChar(UUID_VELOCITY_DATA, BLENotify, PAYLOAD_SIZE); // Velocity history data characteristic (last 50 values)

// Pipe diameter service and characteristic (read/write)
BLEService pipeDiameterService(UUID_PIPE_DIAMETER_SVC);
BLEUnsignedIntCharacteristic pipeDiameterChar(UUID_PIPE_DIAMETER, BLERead | BLEWrite);

// Yearly flow service and characteristic
BLEService yearlyFlowService(UUID_YEARLY_FLOW_SVC);
BLECharacteristic yearlyFlowChar(UUID_YEARLY_FLOW, BLENotify, 5);

// Device information service - Version
BLEService versionService(UUID_VERSION_SVC);                    // Device Information Service (0x180A)
BLEStringCharacteristic versionChar(UUID_VERSION, BLERead, 20); // Firmware Revision String (0x2A28)

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
	BLE.setAdvertisedService(flowHistoryService);
	
	// Configure service and characteristic for flow history (last 50 values)
	flowHistoryService.addCharacteristic(flowHistoryChar);
	BLE.addService(flowHistoryService);
	flowHistoryChar.setEventHandler(BLESubscribed, onFlowHistorySubscribe);
	flowHistoryChar.setEventHandler(BLEUnsubscribed, onFlowHistoryUnsubscribe);
	
	// Configure service and characteristic for velocity history (last 50 values)
	velocityHistoryService.addCharacteristic(velocityHistoryChar);
	BLE.addService(velocityHistoryService);
	velocityHistoryChar.setEventHandler(BLESubscribed, 
		[](BLEDevice central, BLECharacteristic ch) {
			velocityHistorySubscribed = true;
			DEBUG_PRINT("Velocity history notifications enabled by central: ");
			DEBUG_PRINTLN(central.address());
		});
	velocityHistoryChar.setEventHandler(BLEUnsubscribed, 
		[](BLEDevice central, BLECharacteristic ch) {
			velocityHistorySubscribed = false;
			DEBUG_PRINTLN("Velocity history notifications disabled");
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

void onFlowHistorySubscribe(BLEDevice central, BLECharacteristic characteristic)
{
	flowHistorySubscribed = true;
	DEBUG_PRINT("Flow history notifications enabled by central: ");
	DEBUG_PRINTLN(central.address());
}

void onFlowHistoryUnsubscribe(BLEDevice central, BLECharacteristic characteristic)
{
	flowHistorySubscribed = false;
	DEBUG_PRINTLN("Flow history notifications disabled");
}

bool bleSendFlowHistoryBuffer(const uint16_t *samples, int length)
{
	if (!flowHistorySubscribed)
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
	return flowHistoryChar.writeValue(payload, HEADER_SIZE + length * SAMPLE_SIZE);
}

bool bleSendVelocityHistoryBuffer(const uint16_t *samples, int length)
{
	if (!velocityHistorySubscribed)
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
	return velocityHistoryChar.writeValue(payload, HEADER_SIZE + length * SAMPLE_SIZE);
}

bool bleSendYearlyFlow()
{
	if (!yearlyFlowSubscribed)
		return false;
	
        uint8_t payload[5];
        payload[0] = PAYLOAD_VERSION;
        payload[1] = sensor_data.yearly_flow >> 24;
        payload[2] = (sensor_data.yearly_flow >> 16) & 0xFF;
        payload[3] = (sensor_data.yearly_flow >> 8) & 0xFF;
        payload[4] = sensor_data.yearly_flow & 0xFF;
	return yearlyFlowChar.writeValue(payload, 5);
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
