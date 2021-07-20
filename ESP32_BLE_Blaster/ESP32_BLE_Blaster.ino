/*
 *    Created by: Andrew O'Shei
 * 
 *      ESP32 BLE Blaster
 *      Bluetooth LE Testing Platform
 *      w / AES128-GCM Encryption
 * 
 */

#include "oled_interface.h"
#include "sd_interface.h"
#include "ble_interface.h"
#include "button_manager.h"
#include "gcm_encrypt.h"

// Charge Pin, Logic 1 when battery is charging
#define CHARGE  4

// Set name for Bletooth LE Device here
std::string bleName = "BLE Blaster";

// Root path
String root_dir = "/BLE_Blaster";
String devices_file_path = "/BLE_Blaster/BLE_Devices.json";
String file_device = "BLE_Devices.json";
String file_messages = "BLE_Recvd_Text.log";


// SETUP AES Encryption ---------------------- //
// Set component's max size
#define TAG_SIZE  16
#define IV_SIZE   12
size_t outputSize = 0;

// Encryption and Decryption Key
const uint8_t SECRET_KEY[16] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x6d, 0x79,
                                0x20, 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64, 0x21};
// ------------------------------------------- //


// Message received buffer
const size_t BUFF_SIZE = 512;
byte * mBuffer;
String rpText = "";

// Connection tracker
bool connTracker = false;
// Main Screen indicators
int btMode = 0;
bool btRecvd = false;
// Delay to release received message indicator
long btDelay = 0;
// Counts valid messages received
int cntRecvd = 0;
// Counts devices discovered
int cntDevice = 0;

// Sets the current screen mode, 0 = Main
//                               1 = Scan Select
//                               2 = Scan Warning
//                               3 = Scanning
//                               4 = Del Device Warning
int currentScreen = 0;

// Set cursor position on dScanIndex
int dIndex = 1;

// Check if file exists, write file if not found ---------- //
void verifyFile(String file, String initText) {
  String path = root_dir + "/" + file;
  writeLog(file);
  if (checkPathExists(path)) {
    writeLog("Found!");
  } else {
    if (writeFile(path, initText)) {
      writeLog("Created!");
    } else {
      writeLog("Failed to write:");
      writeLog(file);
    }
  }
}



// Get this App off the ground
void setup() {
    Serial.begin(115200);
    pinMode(CHARGE, INPUT);
    delay(500);

    // Start OLED Display -------- //
    initOLED();

    // Check if Charging --------- //
    while(digitalRead(CHARGE) == HIGH) {
        animateCharging();
        delay(250);
    }

    // Setup button pins
    buttonSetup();
    
    // Display Splash Screen ----- //
    splashOLED();

    // Init SD Card Reader ------- //
    writeLog("Init SD Reader...");
    delay(250);
    // Sometimes SD doesn't init right away
    // so we try a few times before giving up
    int wait = 5;
    while (1) {
        if (initSDCard()) {
             writeLog("SD Card Found!");
             break;
        }
        if (!wait) {
            writeLog("No SD Card Found.");
            break;
        }
        wait -= 1;
        delay(1000);
    }
    delay(250);

    // CHECK Root Dir ------------ //
    writeLog("Checking Root Dir...");
    delay(250);
    if (checkPathExists(root_dir)) {
        writeLog("Root Dir Found!");
    } else {
        if (createDir(root_dir)) {
            writeLog("Root Dir Created!");
        } else {
            writeLog("Error: Mkdir failed.");
        }
    }
    delay(250);

    verifyFile(file_device, " ");
    delay(250);
    verifyFile(file_messages, " ");
    delay(250);
    
    // Start BLE Services -------- //
    writeLog("Init BLE Interface...");
    delay(250);
    initBluetoothLE(bleName);
    writeLog("BLE Started!");
    delay(250);
    writeLog("Advertising!");
    delay(250);
    String dName = bleName.c_str();
    writeLog("Name: " + dName);
    writeLog("MAC " + getOwnAddress());
    delay(250);
    writeLog("Init encryption");
    initCipher(SECRET_KEY, IV_SIZE, TAG_SIZE, &outputSize);
    delay(250);
    writeLog("Starting...");
    delay(1000);
}


void loop() {
    // Check if button is pressed
    int button = updateButtons();

    // Update indicators
    btMode = getBLEStatus();
    // Update recvd message flag
    recvdFlagManager();
    
    // TODO Add functions to deal with messages here
    if (getMessageStatus()) {
        mBuffer = getBLEMessage();
        // Parse recvd message here, then decrypt if valid
        if (prepareDecrypt(mBuffer)) {
            rpText = getDecryptedText();
            cntRecvd += 1;
            writeChat(rpText);
            Serial.println(rpText);
        }
        // Send ACK 
        uint8_t * ack = encryptMessage("\6ACK");
        sendBLEMessage(ack, outputSize);
    }

    // In case of disconnect reset counters
    if (!connTracker) {
        if (getIsConnected()) {
            cntRecvd = 0;
            resetChat();
            connTracker = true;
        }
    }
    if (connTracker && !getIsConnected()) {
        connTracker = false;
        resetCounters();
    }


    // Update screens and buttons
    switch(currentScreen) {
        // Scan select Screen ---------------------------------------//
        case 1:
            ScanSelManager(button);
            break;
        // Scan warning screen --------------------------------------//
        case 2:
            break;
        // Scanning animation screen --------------------------------//
        case 3:
            AnimScanManager(button);
            break;
        // Del Device warning screen --------------------------------//
        case 4:
            WarnDelDevManager(button);
            break;
        // Del Device warning screen --------------------------------//
        case 5:
            SenderManager(button);
            break;
        // Primary Display Screen -----------------------------------//
        default:
            MainManager(button);
    }
}


void recvdFlagManager() {
    if (!btRecvd) {
        btRecvd = getBLERecvd();
    } else {
        // Set timer to release btRecvd
        if (btDelay == 0) {
            btDelay = millis();
        }
        // Release btRecvd Here
        else if((millis() - btDelay) > 30) {
            btRecvd = false;
            btDelay = 0;
        }
    }
}


void MainManager(int bVal) {
    int mInd = 0;
    if (bVal == 1) {
        mInd = -1;
    }
    if (bVal == 2) {
        mInd = 1;
    }
    if (bVal == 3) {
        // Go to Config / Scripts menu             
    }
    if (bVal == 4) {
        // Go to Scan select screen
        currentScreen = 1;
        return;
    }
    if (bVal == 5) {
        if (getIsConnected()) {
            currentScreen = 5;
            return;
        } else {
            displayWarning("Must be connected", "to send messages.", btMode, btRecvd, cntRecvd);
            delay(2000);
        }
    }
    displayMain(mInd, btMode, btRecvd, cntRecvd);
}

void SenderManager(int bVal) {
    int row, col;
    bool edit = false;
    row = 0;
    col = 0;
    if (bVal == 1) {
        row = -1;
    }
    if (bVal == 2) {
        row = 1;
    }
    if (bVal == 3) {
        col = -1;
    }
    if (bVal == 4) {
        col = 1;
    }
    if (bVal == 5) {
        if (getSenderRow() < 2) {
            edit = true;
        } else {
            int sel = getSenderSelection();
            // Send message here
            if (sel == 1) {
                String sText = getSenderMessage();
                Serial.print("Message: ");
                Serial.println(sText);
                uint8_t * outText = encryptMessage(sText);
                sendBLEMessage(outText, outputSize);
                btRecvd = true;
            }
            if (sel == 2) {
                resetSender();
            }
            if (sel == 3) {
                resetSender();
                currentScreen = 0;
                return;
            }
        }
    }
    displaySender(edit, row, col, btMode, btRecvd, cntRecvd);
}


void ScanSelManager(int bVal) {
    if (bVal == 1) {
        dIndex = 1;
    }
    if (bVal == 2) {
        dIndex = 2;
    }
    if (bVal == 3) {
        // Go to Scan select screen
        currentScreen = 0;
        return;           
    }
    if (bVal == 4) {
        // Does nothing for now
    }
    if (bVal == 5) {
        if (dIndex == 1) {
            // Scan for BLE Devices
            currentScreen = 3;
            scanBluetoothLE();
            return;
        }
        // Go to delete device warning
        else {
            dIndex = 1;
            currentScreen = 4;
            return;                   
        }
    }
    displayScan(dIndex, btMode, btRecvd, cntRecvd);
}


void AnimScanManager(int bVal) {
    if (bVal) {
        scanStopScan();
        while (scanIsScanning()) { delay(10); }
        writeDeviceJson(getScannedDevices(), devices_file_path);
        String str2 = "Found: " + (String)cntDevice;
        displayWarning("Scan Complete!", str2, btMode, btRecvd, cntRecvd);
        delay(2500);
        currentScreen = 1;
        return;
    }
    // Get device count here
    cntDevice = scanGetCount();
    animateScan(cntDevice);
}


void WarnDelDevManager(int bVal) {
    if (bVal == 1) {
        dIndex = 1;
    }
    if (bVal == 2) {
        dIndex = 2;
    }
    if (bVal == 5) {
        if (dIndex == 1) {
            if (deleteFile(devices_file_path)) {
                displayWarning("Scanned devices", "cleared.", btMode, btRecvd, cntRecvd);
                delay(2500);
            } else {
                displayWarning("Failed to clear", "devices.", btMode, btRecvd, cntRecvd);
                delay(2500);
            }
            writeFile(devices_file_path, " ");
            dIndex = 1;
            currentScreen = 1;
            return;
        }
        else {
            dIndex = 1;
            currentScreen = 1;
            return;
        }
    }
    delDevWarning(dIndex, btMode, btRecvd, cntRecvd);
}
