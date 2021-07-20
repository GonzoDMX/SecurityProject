/*
 *    Created by: Andrew O'Shei
 * 
 *    BLE UART Service Manager
 * 
 */


#ifndef BLE_INTERFACE_H
#define BLE_INTERFACE_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>

#include "ble_appearance.h"

// Init Bluetooth LE -------------------- //
void initBluetoothLE(const std::string btName);
// -------------------------------------- //

// Get Connect Status ------------------- //
int getBLEStatus();
bool getBLERecvd();
bool getIsConnected();
// -------------------------------------- //

// BLE Message Operations --------------- //
bool getMessageStatus();
byte * getBLEMessage();
bool sendBLEMessage(uint8_t* sendArray, size_t oSize);
// -------------------------------------- //

// Scan BLE Devices -------------------- //
void scanBluetoothLE();
void scanStopScan();
bool scanIsScanning();
int scanGetCount();
BLEScanResults getScannedDevices();
// ------------------------------------- //

String getOwnAddress();

#endif
