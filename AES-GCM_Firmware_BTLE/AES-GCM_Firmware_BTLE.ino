/*
 *    Created by: Andrew O'Shei
 * 
 *    Bluetooth LE UART Service
 *    w / AES128-GCM Encryption
 * 
 */

/* TODO MUST FIX COUNTER BEFORE TESTING WITH APP */
 
#include <string.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "gcm_encrypt.h"
#include "seven_seg.h"
#include "button_manager.h"
#include "color_click.h"

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif


// SETUP AES Encryption ----------------------
// Set component's max size
#define TAG_SIZE  16
#define IV_SIZE   12
size_t outputSize = 0;
// Encryption and Decryption Key
const uint8_t SECRET_KEY[16] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x6d, 0x79,
                                0x20, 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64, 0x21};
                          
// Holds first three bytes from mesage in order to determine Integer value
char sizeBuffer[3];
// Hold plaintext messages
String btBuffer;
// If true there is a message ready to encrypt
bool encryptMe = false;
// ------------------------------------------- //


// SET DISPLAY FLAGS ---------- //
bool notify_flag = false;
int notify_select = 0;
unsigned long notify_delay = 0;
// -------------------------- //


// Setup Bluetooth Low Energy ---------------- //
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = true;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Device connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Device disconnect");
      deviceConnected = false;
    }
};

int packetLen = 0;
// Used to catch messages that are longer than one packet
bool rollOver = false;  // If a message is longer than 506 chars (509 incl. length)
int rollLen = 0;        // Holds length of remaining message

// Set max message packet size
const int maxPacketSize = 506;

byte * bt_array;
int bt_size;

bool decryptIn = false;

int getBtSize(char* buf) {
  return ((buf[0] - 48) * 100) + ((buf[1] - 48) * 10) + (buf[2] - 48);
}

// Note max message size allowed is 509 characters
// then it will wrap to the next message
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      
      packetLen = rxValue.length();

      if (rollOver) {
        if (packetLen >= rollLen) {
          for (int i = 0; i < rollLen; i++) {
            bt_array[i + maxPacketSize] = rxValue[i];
          }
          decryptIn = true;
        }
        rollOver = false;
      } else {
        Serial.println("-----------------------------");
        // Read size from uart service input
        if (packetLen > 3) {
          for (int i = 0; i < 3; i++) {
            sizeBuffer[i] = rxValue[i];
          }
          // Get the message size
          int mSize = getBtSize(sizeBuffer);
          Serial.print("Message size: ");
          Serial.println(mSize);

          // Print raw message received for DEBUG
          Serial.print("Raw Message: ");
          for (int i = 0; i < rxValue.length(); i++) {
            Serial.print(rxValue[i]);
          }
          Serial.println();

          btBuffer = rxValue.c_str();
          encryptMe = true;
          
          // If message is too small abort message
          if (mSize < 29 || packetLen < 32) {
            notify_select = 3;
            notify_flag = true;
            return;
          }
          
          // Clear message array
          bt_array = (byte*)malloc(sizeof(byte)*mSize);
          bt_size = mSize;
          
          // Check size
          // If message rolls over to next packet
          if (mSize > maxPacketSize) {
            rollLen = mSize - maxPacketSize;
            mSize = maxPacketSize;
            rollOver = true;
          } else {
            // Message is contained in packet decrypt immediately
            decryptIn = true;
          }
          // Write the RX message to buffer
          for (int i = 0; i < mSize; i++) {
            bt_array[i] = rxValue[i];
          }
        }
      }
      // If the complete message has been received decrypt
      if(decryptIn) {
        Serial.println("Decrypting...");
        btBuffer = decryptMessage(bt_array, bt_size);
        free(bt_array);
        incrementCount();      // Update message received counter on 7 Segment Display
        if (btBuffer.equals("")) {
          Serial.println("Error, Decrypt failed!");
          notify_select = getNotify();
          notify_flag = true;
        } else {
          Serial.println("Finished Decrypting!");
          // Validate the message has the correct count
          if (validateCounter()) {
            Serial.println("Counter validated!");
            Serial.print("Message received: ");
            Serial.println(btBuffer);
            encryptMe = true;
          } else {
            // Invalid counter value
            notify_select = 4;
            notify_flag = true;
          }
        }
      }
    }
};
// ------------------------------------------- //


void setup() {
  // Start UART Serial interface
  Serial.begin(115200);

  delay(1000);

  // Start Bluetooth LE Service
  BLEDevice::init("UART Service");

  // Create BTLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BTLE Service
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
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  // --------------------------------------------- //

  // Setup Cipher
  setupCipher(SECRET_KEY, IV_SIZE, TAG_SIZE, &outputSize);
  // Setup TCS3471 Color Click
  setupTCS3471();
  // Setup Push buttons and LEDs
  setupLedButtons();
  // Setup 7 Seg Display Pins
  setupSevSeg();


  // ---------- BRING BLUETOOTH ONLINE ---------- //
  // Start the Service
  pService->start();
  // Start Advertising
  //pServer->getAdvertising()->start();
  // -------------------------------------------- //
}


void loop() {

  // Check for button press
  updateButtons();

  
  // If device is Connected check to send
  if (deviceConnected) {
    if (encryptMe) {
      Serial.println("-----------------------------");
      Serial.println("Message ready to send.");
      Serial.print("Message: ");
      Serial.println(btBuffer);
      Serial.println("Encrypting...");
      byte* testMessage = encryptMessage(btBuffer);

      // Check that message passed the size check
      if (outputSize > 0) {
        size_t oSize = outputSize + 3;
        crypto_feed_watchdog();

        Serial.println("Encryption finished!");
        Serial.println("Sending message...");

        uint8_t * outputBuffer = (uint8_t*)malloc(sizeof(uint8_t)*oSize);
        
        // Send the message size
        outputBuffer[0] = ((outputSize / 100) % 10) + 48;
        outputBuffer[1] = ((outputSize / 10) % 10) + 48;
        outputBuffer[2] = (outputSize % 10) + 48;

        for (int i = 0; i < outputSize; i++) {
          outputBuffer[i+3] = testMessage[i];
        }
        pTxCharacteristic->setValue(outputBuffer, oSize);
        pTxCharacteristic->notify();
        free(outputBuffer);
    } else {
      // Clear message
      btBuffer = "";
      // Notify size error
      notify_select = 3;
      notify_flag = true;
    }
      // Clear the message
      btBuffer = "";
      encryptMe = false;
      delay(10); // bluetooth stack will go into congestion, if too many packets are sent
    }
  }

  
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }


  // -- Update the seven segment display -- //
  if (notify_flag) {
    updateDisp(notify_select);
    if (!notify_delay) {
      notify_delay = millis();
    }
    if (millis() - notify_delay > 3000) {
      notify_flag = false;
      notify_delay = 0;
    }
  } else {
    countDisplay();
  }
  // ------------------------------------ //
}


bool validateCounter() {
    byte r = get_rCount();
    byte m = get_mCount();
    if (r == 0x00 && btBuffer.equals("<CONNECT>")) {
      Serial.println("Bluetooth Connected!");
      set_mCount(0x00);
      notify_select = 1;
      notify_flag = true;
      return true;
    } else {
      // Invalid message purge
      btBuffer = "";
      return false;
    }
    if (r == 0x00 && btBuffer.equals("<DISCONN>")) {
      Serial.println("Bluetooth Disconnected!");
      set_mCount(0x01);
      notify_select = 2;
      notify_flag = true;
      return true;
    } else if (r == m) {
      if (m == 0xFF) {
        set_mCount(0x01);
      } else {
        incr_mCount();
      }
      return true;
    } else {
      btBuffer = "";
      return false;
    }
}
