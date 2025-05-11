#include <ArduinoBLE.h>
#include <Arduino.h>
#include <WiFi.h>

typedef struct {
  uint16_t flow_rate;    // Water flow rate (divide by 100 to get actual value)
} sensor_data_t;

sensor_data_t sensor_data;

// BLE service UUIDs - Using the same UUIDs as your original code to ensure compatibility
#define UUID_FLOW_SVC           "181A"  // Environmental Sensing service (standard)
#define UUID_DATA_SVC           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // Original Nordic UART Service UUID
#define UUID_VERSION_SVC        "180A"  // Device Information service (standard)

// BLE characteristic UUIDs
#define UUID_FLOW               "2A6D"  // Standard "Flow" characteristic
#define UUID_DATA               "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Original Nordic UART TX characteristic
#define UUID_VERSION            "2A28"  // Software Revision String (standard)

char device_name[32];

// BLE services and characteristics
BLEService flowService(UUID_FLOW_SVC);
BLEUnsignedIntCharacteristic flowChar(UUID_FLOW, BLERead);

BLEService dataService(UUID_DATA_SVC);
BLECharacteristic dataChar(UUID_DATA, BLENotify, 3);  // 3 bytes: 1 reserved + 2 for flow

BLEService versionService(UUID_VERSION_SVC);
BLEStringCharacteristic versionChar(UUID_VERSION, BLERead, 20);

const char* VERSION = "1.0.0";
bool subscribed = false;
unsigned long lastUpdateTime = 0;
const long updateInterval = 1000;

// Generate unique device ID using MAC address
uint64_t getDeviceId() {
  uint64_t id = 0;
  byte mac[6];
  WiFi.macAddress(mac);
  for (int i = 0; i < 6; i++) {
    id = (id << 8) | mac[i];
  }
  return id;
}

void startAdvertising() {
  BLE.advertise();
  Serial.println("Started advertising");
}

void bleSetup() {
  uint64_t device_id = getDeviceId();
  
  sprintf(device_name, "FLOW-%08llX", device_id);
  
  if (!BLE.begin()) {
    Serial.println("BLE initialization failed!");
    while (1);
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
  
  Serial.println("BLE initialized");
  Serial.print("Device ID: ");
  Serial.println(device_id, HEX);
  Serial.print("Device Name: ");
  Serial.println(device_name);
}

void onSubscribe(BLEDevice central, BLECharacteristic characteristic) {
  subscribed = true;
  Serial.print("Notifications enabled by central: ");
  Serial.println(central.address());
}

void onUnsubscribe(BLEDevice central, BLECharacteristic characteristic) {
  subscribed = false;
  Serial.println("Notifications disabled");
}

void buildSensorDataBuffer(const sensor_data_t *data, uint8_t *buf) {
  buf[0] = 0;  // Reserved byte
  buf[1] = (data->flow_rate & 0xFF00) >> 8;  // Flow rate MSB
  buf[2] = (data->flow_rate & 0xFF);         // Flow rate LSB
}

boolean bleSendData() {
  if (!subscribed) {
    return false;
  }
  
  uint8_t buffer[3];  // 3 bytes: 1 reserved + 2 for flow rate
  buildSensorDataBuffer(&sensor_data, buffer);
  
  return dataChar.writeValue(buffer, 3);
}

void updateSensorData() {

  // TODO CJS -> Implement sensor reading.
  double flowValue = 10;
  
  // Convert to uint16_t format (multiply by 100 to preserve 2 decimals)
  uint16_t scaledFlow = (uint16_t)(flowValue * 100);
  
  sensor_data.flow_rate = scaledFlow;
  
  flowChar.writeValue(sensor_data.flow_rate);
  
  Serial.print("Flow rate: ");
  Serial.print(sensor_data.flow_rate);
  Serial.println(" (raw value)");
  Serial.print("Actual flow: ");
  Serial.print(flowValue);
  Serial.println(" L/min");
}

void setup() {
  Serial.begin(9600);
  delay(1500);
  
  // TODO CJS -> Manage corecctly
  pinMode(A0, INPUT);
  
  sensor_data.flow_rate = 0;
  
  bleSetup();
  
  Serial.println("Setup complete");
}

void loop() {
  unsigned long currentTime = millis();
  
  BLEDevice central = BLE.central();
  
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    
    while (central.connected()) {
      currentTime = millis();
      if (currentTime - lastUpdateTime >= updateInterval) {
        lastUpdateTime = currentTime;
        
        updateSensorData();
        
        if (subscribed) {
          if (bleSendData()) {
            Serial.println("Data sent successfully");
          } else {
            Serial.println("Failed to send data");
          }
        }
      }
      
      delay(10);
    }
    
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    
    subscribed = false;
    
    startAdvertising();
  } 

  delay(100); // wait on new connection
}