/*
 *    Created by: Andrew O'Shei
 * 
 *    BLE UART Service Manager
 * 
 */

#include "ble_interface.h"


BLEScan* pBLEScan;
BLEScanResults foundDevices;
BLEAdvertising *pAdvertising;
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;

// Set BLE Service UUIDs -------------- //
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
// ------------------------------------ //

// SCAN RELATED VARIABLES ------------- //
// Set Scan Timeout
const int scanTime = 60;
// Set flag if Scanning
bool isScanning = false;
// Count the number of devices scanned
int scanDeviceCount = 0;
// ------------------------------------ //


// CONNECT AND MESSAGE VARIABLES ------ //
// Set status indicator, 0 = Offline, 1 = Advertising, 2 = Connected, 3 = Disconnect, 4 = Scanning
int bleStatus = 0;
// Set true if BLE is connected to a device
bool deviceConnected = false;
// Did anything trigger a receive?
bool trigReceived = false;
// Set true if a message is available
bool isThereMessage = false;
// Set true if BLE is Advertising
bool isAdvertising = false;

// Set Maximum packet size that can be received
const int maxPacketSize = 512;
// Set Maximum packet size that can be received
const int minPacketSize = 3;
// Message buffer for storing messages
byte recvdArray[512];
// ------------------------------------ //


// Callback for Connection Events ------------ //
class ConnectCallback: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Device connect");
        deviceConnected = true;
        bleStatus = 2;
    };
    
    void onDisconnect(BLEServer* pServer) {
        Serial.println("Device disconnect");
        deviceConnected = false;
        bleStatus = 3;
        delay(1000);
        pAdvertising->start();
        isAdvertising = true;
        bleStatus = 1;
    }
};
// ------------------------------------------- //


// Receive Messages on BLE ------------------- //
class ReadCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        
        int packetLen = rxValue.length();
        Serial.println("Message Received");
        trigReceived = true;
        // If message is too big
        if (packetLen > maxPacketSize) {
            Serial.println("Packet oversized.");
            return;
        }
        else if (packetLen < minPacketSize) {
            Serial.println("Packet undersized.");
            return;
        }
        else {
            if (!isThereMessage) {
                memset(recvdArray, 0x00, 512);
                for (int i = 0; i < packetLen; i++) {
                    recvdArray[i] = rxValue[i];
                }
                isThereMessage = true;
            } else {
                Serial.println("Error: Message overloaded!");
            }
        }
    }
};
// ------------------------------------------------- //


// ------------------------------------------------- //
// Get BLE Connection Status ----------------------- //
int getBLEStatus() {
    return bleStatus;
}

// Get Received flag for Indicator
bool getBLERecvd() {
    if (trigReceived) {
        trigReceived = false;
        return true;
    } else {
        return false;
    }
}

// Get true if connected
bool getIsConnected() {
    return deviceConnected;
}


// Get Own BLE Address
String getOwnAddress() {
    BLEAddress bleAddress = BLEDevice::getAddress();
    return bleAddress.toString().c_str();
}
// ------------------------------------------------- //


// BLE Message Send and Receive -------------------- //
bool getMessageStatus() {
    return isThereMessage;
}

byte * getBLEMessage() {
    if (isThereMessage) {
        isThereMessage = false;
        return recvdArray;
    } else {
        Serial.println("No Message available");
        return { 0x00 };
    }
}

bool sendBLEMessage(uint8_t* sendArray, size_t oSize) {
    if (deviceConnected) {
        pTxCharacteristic->setValue(sendArray, oSize);
        pTxCharacteristic->notify();
        return true;
    } else {
        Serial.println("Cannot send message device not connected");
        return false;
    }
}
// ------------------------------------------------- //


// BLE Scan Operations ----------------------------- //
// Class for BLE Scanning Function ----------------- //
class ScanCallback: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
        scanDeviceCount += 1;
    }
};

// Called as a FreeRTOS Task from scanBluetoothLE()
void startBLEScan(void *params) {
    // If device is advertising stop
    if (isAdvertising) {
        pAdvertising->stop();
    }

    // bleStatus = 4;
    foundDevices = pBLEScan->start(scanTime, false);
    isScanning = false;
    vTaskDelete(NULL);
}

void scanBluetoothLE() {
    isScanning = true;
    pBLEScan->clearResults();
    xTaskCreate(startBLEScan, "Scan BLE", 1000, NULL, 1, NULL);
}

void scanStopScan() {
    pBLEScan->stop();
}

bool scanIsScanning() {
    return isScanning;
}

int scanGetCount() {
    return scanDeviceCount;
}

BLEScanResults getScannedDevices() {
    return foundDevices;
}
// ------------------------------------------------ //



// INIT Bluetooth LE ------------------------------- //
void initBluetoothLE(const std::string btName) {
    BLEDevice::init(btName);
    
    // Create BLE Server ---------- //
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ConnectCallback());

    // Create BLE Scanner --------- //
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new ScanCallback());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    // Create BTLE Service -------- //
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create Tx (Transmit) BTLE Service
    pTxCharacteristic = pService->createCharacteristic(
                                    CHARACTERISTIC_UUID_TX,
                                    BLECharacteristic::PROPERTY_NOTIFY
                                    );

    pTxCharacteristic->addDescriptor(new BLE2902());

    // Create Rx (Receive) BTLE Service
    BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                                                        CHARACTERISTIC_UUID_RX,
                                                        BLECharacteristic::PROPERTY_WRITE
                                                        );
                                                        
    pRxCharacteristic->setCallbacks(new ReadCallback());
    
    // BRING BLUETOOTH ONLINE ----- //
    // Start the Service
    pService->start();
    // Start Advertising
    pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
    isAdvertising = true;
    bleStatus = 1;
}
// ------------------------------------------------- //
