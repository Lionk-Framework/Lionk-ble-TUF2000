#include "ble_manager.h"
#include "sensor.h"
#include "utils.h"

constexpr char UUID_FLOW_SVC[] = "181A";
constexpr char UUID_DATA_SVC[] = "19B10000-E8F2-537E-4F6C-D104768A1214";
constexpr char UUID_VERSION_SVC[] = "180A";
constexpr char UUID_FLOW[] = "2A6D";
constexpr char UUID_DATA[] = "19B10001-E8F2-537E-4F6C-D104768A1214";
constexpr char UUID_VERSION[] = "2A28";

char device_name[32];
constexpr const char *VERSION = "1.0.0";
bool subscribed = false;

BLEService flowService(UUID_FLOW_SVC);
BLEUnsignedIntCharacteristic flowChar(UUID_FLOW, BLERead);
BLEService dataService(UUID_DATA_SVC);
BLECharacteristic dataChar(UUID_DATA, BLENotify, PAYLOAD_SIZE);
BLEService versionService(UUID_VERSION_SVC);
BLEStringCharacteristic versionChar(UUID_VERSION, BLERead, 20);

void startAdvertising()
{
	BLE.advertise();
	DEBUG_PRINTLN("Started advertising");
}

void bleSetup()
{
	uint64_t device_id = getDeviceId();
	snprintf(device_name, sizeof(device_name), "FLOW-%08llX", device_id);
	if (!BLE.begin()) {
		DEBUG_PRINTLN("BLE initialization failed!");
		while (true)
			;
	}
	BLE.setDeviceName(device_name);
	BLE.setLocalName(device_name);
	BLE.setAdvertisedService(dataService);
	flowService.addCharacteristic(flowChar);
	dataService.addCharacteristic(dataChar);
	versionService.addCharacteristic(versionChar);
	BLE.addService(flowService);
	BLE.addService(dataService);
	BLE.addService(versionService);
	flowChar.writeValue(sensor_data.flow_rate);
	versionChar.writeValue(VERSION);
	dataChar.setEventHandler(BLESubscribed, onSubscribe);
	dataChar.setEventHandler(BLEUnsubscribed, onUnsubscribe);
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

bool bleSendData()
{
	if (!subscribed)
		return false;
	uint8_t buffer[PAYLOAD_SIZE];
	buildSensorDataBuffer(&sensor_data, buffer);
	return dataChar.writeValue(buffer, PAYLOAD_SIZE);
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
