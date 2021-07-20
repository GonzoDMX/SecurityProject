/*
 *    Created by: Andrew O'Shei
 * 
 *    Various functions to write and retrieve data from SD Card Reader
 * 
 */

#include "sd_interface.h"

DynamicJsonDocument deviceJson(4096);

char path[256];

bool initSDCard() {
    if (!SD.begin()) {
        return false;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        return false;
    }
    return true;
}

void setPath(String targ_path) {
    memset(path, 0x00, sizeof(path));
    for (int i = 0; i < targ_path.length(); i++) {
        path[i] = (char)targ_path[i];
    }
}


bool createDir(String targ_path) {
    setPath(targ_path);
    if(SD.mkdir(path)){
        return true;
    }
    return false;
}


bool writeFile(String targ_path, String message) {
    setPath(targ_path);
    File file = SD.open(path, FILE_WRITE);
    if(!file.print(message)){
        return false;
    }
    file.close();
    return true;
}


bool writeDeviceJson(BLEScanResults devices, String targ_path) {
    if (!deleteFile(targ_path)) {
        Serial.println("Error clearing old devices.");
    }
    Serial.println("Compiling devices to JSON");
    int dCount = devices.getCount();
    for (int i = 0; i < dCount; i++) {
        BLEAdvertisedDevice dev = devices.getDevice(i);
        String id = (String)(i + 1);
        // Set Device Name
        String devName = dev.getName().c_str();
        if (devName.equals("")) {
            deviceJson["devices"][id]["name"] = "UnNamed";
        } else {
            deviceJson["devices"][id]["name"] = devName;
        }
        deviceJson["devices"][id]["appearance"] = getBLEAppearance(dev.getAppearance());
        deviceJson["devices"][id]["address"] = dev.getAddress().toString();
        deviceJson["devices"][id]["rssi"] = (String)dev.getRSSI();
    }
    String output;
    serializeJsonPretty(deviceJson, output);
    if (!writeFile(targ_path, output)) {
        Serial.println("Failed to write devices");
        return false;
    }
    return true;
}


bool deleteFile(String targ_path) {
    setPath(targ_path);
    if(!SD.remove(path)){
        return false;
    }
    return true;
}

bool checkPathExists(String targ_path) {
    if (SD.exists(getCharArray(targ_path))) {
        return true;    
    }
    return false;
}
