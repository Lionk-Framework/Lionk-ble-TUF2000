#include <ArduinoBLE.h>
#include <Arduino.h>
#include <WiFi.h>

// Mode debug - mettre à false pour désactiver tous les Serial.print
#define DEBUG_MODE false

typedef struct {
  uint16_t flow_rate;    // Water flow rate (divide by 100 to get actual value)
} sensor_data_t;

sensor_data_t sensor_data;

#define PAYLOAD_VERSION 0
#define HEADER_SIZE     2
#define DATA_SIZE       2
#define PAYLOAD_SIZE    HEADER_SIZE + DATA_SIZE

// BLE service UUIDs
#define UUID_FLOW_SVC           "181A"  // Environmental Sensing service (standard)
#define UUID_DATA_SVC           "19B10000-E8F2-537E-4F6C-D104768A1214"  // Custom flow notification service
#define UUID_VERSION_SVC        "180A"  // Device Information service (standard)

// BLE characteristic UUIDs
#define UUID_FLOW               "2A6D"  // Standard "Flow" characteristic
#define UUID_DATA               "19B10001-E8F2-537E-4F6C-D104768A1214"  // Custom flow notification characteristic
#define UUID_VERSION            "2A28"  // Software Revision String (standard)

char device_name[32];

// BLE services and characteristics
BLEService flowService(UUID_FLOW_SVC);
BLEUnsignedIntCharacteristic flowChar(UUID_FLOW, BLERead);

BLEService dataService(UUID_DATA_SVC);
BLECharacteristic dataChar(UUID_DATA, BLENotify, PAYLOAD_SIZE);  // 4 bytes: 2 reserved + 2 for flow

BLEService versionService(UUID_VERSION_SVC);
BLEStringCharacteristic versionChar(UUID_VERSION, BLERead, 20);

const char* VERSION = "1.0.0";
bool subscribed = false;
unsigned long lastUpdateTime = 0;
const long updateInterval = 10;

// Macro pour les messages de debug
#define DEBUG_PRINT(x) if(DEBUG_MODE) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DEBUG_MODE) Serial.println(x)

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
  DEBUG_PRINTLN("Started advertising");
}

void bleSetup() {
  uint64_t device_id = getDeviceId();
  
  sprintf(device_name, "FLOW-%08llX", device_id);
  
  if (!BLE.begin()) {
    DEBUG_PRINTLN("BLE initialization failed!");
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
  
  DEBUG_PRINTLN("BLE initialized");
  DEBUG_PRINT("Device ID: ");
  DEBUG_PRINTLN(device_id, HEX);
  DEBUG_PRINT("Device Name: ");
  DEBUG_PRINTLN(device_name);
}

void onSubscribe(BLEDevice central, BLECharacteristic characteristic) {
  subscribed = true;
  DEBUG_PRINT("Notifications enabled by central: ");
  DEBUG_PRINTLN(central.address());
}

void onUnsubscribe(BLEDevice central, BLECharacteristic characteristic) {
  subscribed = false;
  DEBUG_PRINTLN("Notifications disabled");
}

void buildSensorDataBuffer(const sensor_data_t *data, uint8_t *buf) {
  buf[0] = PAYLOAD_VERSION;  // Reserved byte
  buf[1] = DATA_SIZE;  // Reserved byte
  buf[2] = (data->flow_rate & 0xFF00) >> 8;  // Flow rate MSB
  buf[3] = (data->flow_rate & 0xFF);         // Flow rate LSB
}

boolean bleSendData() {
  if (!subscribed) {
    return false;
  }
  
  uint8_t buffer[PAYLOAD_SIZE];  // 4 bytes: 2 reserved + 2 for flow rate
  buildSensorDataBuffer(&sensor_data, buffer);
  
  return dataChar.writeValue(buffer, PAYLOAD_SIZE);
}

int i = 0;

void updateSensorData() {
  // TODO CJS -> Implement sensor reading.
  double flowValue = 10 + ((i++) % 13);
  
  // Convert to uint16_t format (multiply by 100 to preserve 2 decimals)
  uint16_t scaledFlow = (uint16_t)(flowValue * 100);
  
  sensor_data.flow_rate = scaledFlow;
  
  flowChar.writeValue(sensor_data.flow_rate);
  
  DEBUG_PRINT("Flow rate: ");
  DEBUG_PRINT(sensor_data.flow_rate);
  DEBUG_PRINTLN(" (raw value)");
  DEBUG_PRINT("Actual flow: ");
  DEBUG_PRINT(flowValue);
  DEBUG_PRINTLN(" L/min");
}

void setup() {
  if (DEBUG_MODE) {
    Serial.begin(9600);
  }
  
  // TODO CJS -> Manage correctly
  pinMode(A0, INPUT);
  
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