/*
 *    Created by: Andrew O'Shei
 * 
 *    Various functions to write and retrieve data from SD Card Reader
 * 
 */


#ifndef SD_INTERFACE_H
#define SD_INTERFACE_H

#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include <SPI.h>
#include <ArduinoJson.h>
#include "data_helper.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "ble_appearance.h"


bool initSDCard();

void setPath(String targ_path);

bool createDir(String targ_path);

bool writeFile(String targ_path, String message);

bool writeDeviceJson(BLEScanResults devices, String targ_path);

bool deleteFile(String targ_path);

bool checkPathExists(String targ_path);

#endif
