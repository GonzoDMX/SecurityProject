#include <Wire.h>
#include <TCS3471.h>
#include "BluetoothSerial.h"
#include "esp_gap_bt_api.h"
#include <Crypto.h>
#include <AES.h>
#include <GCM.h>
#include <string.h>
#include <RNG.h>

#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

// SETUP AES Encryption ----------------------
// Set component's max size
#define TEXT_SIZE 64
#define TAG_SIZE  16
#define IV_SIZE   12

// "AES-128 GCM"
GCM<AES128> *gcmaes128 = 0;
byte buffer[128];

size_t outsize = 0;

uint8_t my_key[16] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x6d, 0x79,
                      0x20, 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64, 0x21};
uint8_t plaintext[TEXT_SIZE] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x6d, 0x79,
                                0x20, 0x66, 0x72, 0x69, 0x65, 0x6e, 0x64, 0x21};
uint8_t ciphertext[TEXT_SIZE] = {0x4b, 0xed, 0xb6, 0xa2, 0x0f, 0x96, 0xce, 0xeb,
                                 0xd3, 0x4e, 0xb0, 0xd0, 0x14, 0xdc, 0x9a, 0x59};
uint8_t outgoingIV[IV_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t incomingIV[IV_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t checktag[TAG_SIZE] = {0xbd, 0xde, 0x04, 0x2b, 0x67, 0xc6, 0xdc, 0x59,
                              0xb3, 0xf0, 0xf2, 0x32, 0xc6, 0x71, 0x6f, 0x5c};

char sizeBuffer[3];

String btBuffer;
byte mCounter = 0x01;
byte rCounter = 0x00;
bool encryptMe = false;

bool btConnected = false;

String fail = "";
// ------------------------------------------- //


// SETUP Bluetooth ---------- //
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

char bluetoothBuffer[32];
unsigned long messageCounter = 0;
// -------------------------- //


// SETUP GPIO PINS ---------- //
// Set 7-Seg Display Pins
#define SEG_DIN     19
#define SEG_CS      5
#define SEG_CLK     18

bool notify_flag = false;
int notify_select = 0;

unsigned long notify_delay = 0;

// All hex characters for display 0-F
uint8_t disp_char[16] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47};

// Connect Message
uint8_t notify_connect[8] = {0x4E, 0x7E, 0x76, 0x76, 0x4F, 0x4E, 0x70, 0xC0};
// Disconnect Message
uint8_t notify_disconnect[8] = {0x3D, 0x30, 0x5B, 0x4E, 0x7E, 0x76, 0x76, 0x80};

// Bad Size Error
uint8_t size_error[8] = { 0x7F, 0x77, 0x3D, 0x00, 0x5B, 0x30, 0x6D, 0x4F };
// Counter Position Error
uint8_t pos_error[8] = { 0x7F, 0x77, 0x3D, 0x00, 0x67, 0x7E, 0x5B, 0x00 };
// Bad Tag Error
uint8_t tag_error[8] = { 0x7F, 0x77, 0x3D, 0x00, 0x40, 0x46, 0x77, 0x5E };
// Unknown Error
uint8_t unknown_error[8] = {0x4F, 0x05, 0x05, 0x1D, 0x05, 0x00, 0x7E, 0x00};


// Writes data to the 7 segment display
void setDisplay(byte address, byte value) {
   digitalWrite(SEG_CS, LOW);
   shiftOut(SEG_DIN, SEG_CLK, MSBFIRST, address);
   shiftOut(SEG_DIN, SEG_CLK, MSBFIRST, value);
   digitalWrite(SEG_CS, HIGH);
}

void loadDisplay() {
    uint8_t roll = 0b00000010;
    for(int i = 0; i < 13; i++) {
      if(i == 6) {
        roll = 0b00000010;
      }
      if(i == 12) {
        roll = 0x00;
      }
      for(int i = 8; i > 0; i--){
        setDisplay(i, roll);
      }
      roll = roll << 1;
      delay(100);
    }
}

void zeroDisplay() {
  for (int i = 1; i < 9; i++) {
    setDisplay(i, disp_char[0]);
  }
}

void incrementCount() {
  messageCounter += 1;
  if (messageCounter > 99999999) {
    messageCounter = 0;
  }
}

void countDisplay() {
  int k = 10000000;
  for (int i = 8; i > 0; i--) {
    setDisplay(i, disp_char[messageCounter / k % 10]);
    k = k / 10;
  }
}

// Write Status Messages to 7-Segment Display
void setMessage(byte* message){
  int digit = 8;
  for(int i = 0; i < 8; i++) {
    setDisplay(digit - i, message[i]);
  }
}
// -------------------------- //


// LED STATE Values --------- //
// Set LED GPIO pins
#define LED_RED     17
#define LED_GRN     16
#define LED_BLU     0

// Stores current LED value
int L_RED = 0;
int L_GRN = 0;
int L_BLU = 0;
// -------------------------- //


// SETUP BUTTON States ------ //
// Set button GPIO pins
#define BUTTON_RED  32
#define BUTTON_GRN  33
#define BUTTON_BLU  34
#define BUTTON_BLK  35

// Store current button state
int B_RED = 0;
int B_GRN = 0;
int B_BLU = 0;
int B_BLK = 0;

// Store old button state
int S_RED = 0;
int S_GRN = 0;
int S_BLU = 0;
int S_BLK = 0;

// Store button debounce delay
long D_RED = 0;
long D_GRN = 0;
long D_BLU = 0;
// Set debounce delay time in milliseconds
const long D_DELAY = 150;

// Toggle LED Buttons
void update_button(int button, int *etat, long *bounce, int *led_etat, int led_pin) {
  if( (millis() - *bounce) > D_DELAY) {
    if(button == HIGH && *etat == 0) {
      if(*led_etat == 0) {
        *led_etat = 1;
      } else {
        *led_etat = 0;
      }
      *etat = 1;
      digitalWrite(led_pin, *led_etat);
      *bounce = millis();
    }
    else if(button == LOW && *etat == 1){
      *etat = 0;
      *bounce = millis();
    }
  }
}

// Toggle Black button
void update_blk() {
  if(B_BLK == HIGH && S_BLK == 0) {
    S_BLK = 1;
    L_RED = 0;
    L_GRN = 0;
    L_BLU = 0;
    digitalWrite(LED_RED, L_RED);
    digitalWrite(LED_GRN, L_GRN);
    digitalWrite(LED_BLU, L_BLU);
  }
  else if(B_BLK == LOW && S_BLK == 1){
    S_BLK = 0;
  }
}
// -------------------------- //


// TCS3471 Setup -------------- //
// i2C Read and Write functions
void i2cWrite(byte address, byte count, byte* buffer);
void i2cRead(byte address, byte count, byte* buffer);

// Create TCS3471 Object
TCS3471 TCS3471(i2cWrite, i2cRead);

// TCS3471 chip attached to Arduino's two wire bus
void i2cWrite(byte address, byte count, byte* buffer) {
    Wire.beginTransmission(address);
    byte i;
    for (i = 0; i < count; i++) {
#if ARDUINO >= 100
        Wire.write(buffer[i]);
#else
        Wire.send(buffer[i]);
#endif
    }
    Wire.endTransmission();
}

void i2cRead(byte address, byte count, byte* buffer) {
    Wire.requestFrom(address, count);
    byte i;
    for (i = 0; i < count; i++) {
#if ARDUINO >= 100
        buffer[i] = Wire.read();
#else
        buffer[i] = Wire.receive();
#endif
    }
}
// --------------------------- //


void setup() {
  // Start UART Serial interface
  Serial.begin(115200);

  delay(1000);


  // Start BlueTooth, set device name
  SerialBT.begin("PinTester");

  // Set BT Pin code
  //esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
  //esp_bt_pin_code_t pin_code;
  //pin_code[0] = '1';
  //pin_code[1] = '1';
  //pin_code[2] = '1';
  //pin_code[3] = '1';
  //esp_bt_gap_set_pin(pin_type, 4, pin_code);

  // Start i2C
  Wire.begin();

  // Setup LED pin mode
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GRN, OUTPUT);
  pinMode(LED_BLU, OUTPUT);

  // Set all LEDs to off
  digitalWrite(LED_RED, 0);
  digitalWrite(LED_GRN, 0);
  digitalWrite(LED_BLU, 0);

  // Setup 7 Seg Display Pins
  pinMode(SEG_DIN, OUTPUT);
  pinMode(SEG_CS, OUTPUT);
  pinMode(SEG_CLK, OUTPUT);
  digitalWrite(SEG_CLK, HIGH);
  delay(500);

  // Configure 7 Seg Display
  setDisplay(0x0F, 0x00); // Disable Test Mode
  setDisplay(0x0C, 0x01); // Set Normal Op
  setDisplay(0x0B, 0x07); // Set number of digits 0-7 = 8 in total
  setDisplay(0x0A, 0x0F); // Set Brightness 0F is Max
  setDisplay(0x09, 0x00); // Disable Decode Mode

  loadDisplay();

  /*
  // Check if TCS3471 is connected and functioning
  if(!TCS3471.detect()){
    //Serial.println("Error: Color Click not detected!");
    setMessage(disp_error);
    while(1);
  }
  */
  
  // Setup TCS3471
  // Range is 2.4ms to 614.4ms, more time means slower but more precise
  TCS3471.setIntegrationTime(350.0);
  // Range is 2.4ms to 7400ms, longer waits mean slower reads but lower power consumption
  TCS3471.setWaitTime(150.0);
  // TCS3471_GAIN_1X, TCS3471_GAIN_4X, TCS3471_GAIN_16X and TCS3471_GAIN_60X
  TCS3471.setGain(TCS3471_GAIN_1X);
  // Enable the TCS3471 Chip
  TCS3471.enable();
  //Serial.println("TCS3471 Enabled!");

  // Setup push button pin mode
  pinMode(BUTTON_RED, INPUT);
  pinMode(BUTTON_GRN, INPUT);
  pinMode(BUTTON_BLU, INPUT);
  pinMode(BUTTON_BLK, INPUT);

  zeroDisplay(); 
}



void loop() {
  
  // CHECK FOR BUTTON PRESSES --------- //
  // Read current button states
  B_RED = digitalRead(BUTTON_RED);
  B_GRN = digitalRead(BUTTON_GRN);
  B_BLU = digitalRead(BUTTON_BLU);
  B_BLK = digitalRead(BUTTON_BLK);

  // Check if buttons are pressed
  update_button(B_RED, &S_RED, &D_RED, &L_RED, LED_RED);
  update_button(B_GRN, &S_GRN, &D_GRN, &L_GRN, LED_GRN);
  update_button(B_BLU, &S_BLU, &D_BLU, &L_BLU, LED_BLU);
  update_blk();
  // ---------------------------------- //


  
  // FETCH BLUETOOTH MESSAGE AND DECRYPT ----- //
  if (!encryptMe) {
    if (SerialBT.available() >= 3) {
      Serial.println("-----------------------------");
      Serial.println("Receiving a message...");
      // Get message size
      for (int k = 0; k < 3; k++) {
        sizeBuffer[k] = SerialBT.read();
        // Serial.println(sizeBuffer[k], HEX);
      }
      int mSize = getBtSize(sizeBuffer);
      Serial.print("Message size: ");
      Serial.println(mSize);

      if (mSize > 28 || mSize < 156) {
        // Declare dynamic array and receive the message
        byte* bt_array = (byte *)malloc(sizeof(byte)*mSize);
        for(int x = 0; x < mSize; x++) {
          bt_array[x] = SerialBT.read();
        }

        Serial.println("Decrypting...");
        btBuffer = decryptMessage(bt_array, mSize);
        free(bt_array);
        incrementCount();
        Serial.println("Finished Decrypting!");
        if (validateCounter()) {
          Serial.println("Counter validated!");
          Serial.print("Message received: ");
          Serial.println(btBuffer);
        } else {
          Serial.println("validateCounter() check failed.");
          notify_select = 4;
          notify_flag = true;
        }
      } else {
        // Message is too small dump input buffer -> Bad Size Error
        while (SerialBT.available() > 0) {
          SerialBT.read();
        }
        notify_select = 3;
        notify_flag = true;
      }
    }
  }
  // ---------------------------------- //
  

  // IF MESSAGE AVAILABLE ENCRYPT AND SEND ----- //
  if(!btBuffer.equals("") && btConnected) {
    Serial.println("-----------------------------");
    if (encryptMe) {
      Serial.println("Message ready to send.");
      Serial.print("Message: ");
      Serial.println(btBuffer);
      Serial.println("Encrypting...");
      byte* testMessage = encryptMessage(my_key, outgoingIV, btBuffer);

      // Check that message passed the size check
      if (outsize > 0) {
        crypto_feed_watchdog();

        Serial.println("Encryption finished!");
        Serial.println("Sending message...");
        // Send the message size
        SerialBT.write(((outsize / 100) % 10) + 48);
        SerialBT.write(((outsize / 10) % 10) + 48);
        SerialBT.write((outsize % 10) + 48);
  
        // Send the encrypted message
        for (int i = 0; i < outsize; i++) {
          SerialBT.write(testMessage[i]);
        }
        Serial.println("Message sent!");
        Serial.println("-----------------------------");
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
    } else {
      Serial.println("Update display");
      encryptMe = true;
    }
  }
  // ---------------------------------- //

  if (notify_flag) {
    switch (notify_select) {
      case 1:
        setMessage(notify_connect);
        break;
      case 2:
        setMessage(notify_disconnect);
        break;
      case 3:
        setMessage(size_error);
        break;
      case 4:
        setMessage(pos_error);
        break;
      case 5:
        setMessage(tag_error);
        break;
      default:
        setMessage(unknown_error);
    }
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
}


bool validateCounter() {
  if (!btConnected) {
    if (rCounter == 0x00 && btBuffer.equals("<CONNECT>")) {
      Serial.println("Bluetooth Connected!");
      mCounter = 0x00;
      btConnected = true;
      notify_select = 1;
      notify_flag = true;
      return true;
    } else {
      // Invalid message purge
      btBuffer = "";
      return false;
    }
  } else {
    if (rCounter == 0x00 && btBuffer.equals("<DISCONN>")) {
      Serial.println("Bluetooth Disconnected!");
      btConnected = false;
      mCounter = 0x01;
      notify_select = 2;
      notify_flag = true;
      return true;
    } else if (rCounter == mCounter) {
      if (mCounter == 0xFF) {
        mCounter = 0x01;
      } else {
        mCounter += 1;
      }
      return true;
    } else {
      btBuffer = "";
      return false;
    }
  }
}

uint8_t* encryptMessage(uint8_t *key, uint8_t *iv, String message) {
  // Declare cipher
  AuthenticatedCipher *cipher = new GCM<AES128>();

  // Container for generated tag
  uint8_t tag[16];

  // Indexing variables
  size_t pos, len, inc, off_set = IV_SIZE + TAG_SIZE;
  // 
  size_t datasize = (message.length() + 1);
  inc = datasize;
  outsize = datasize + off_set;
  
  // If message is too big for buffer do not encrypt
  if (outsize > 100) {
    outsize = 0;
    return { 0x00 };
  }

  // Convert message to char array
  plaintext[0] = mCounter;
  for (pos = 1; pos < datasize; pos++) {
    plaintext[pos] = message[pos - 1];
  }
  plaintext[datasize] = 0x00;

  // Increment counter
  if (mCounter == 0xFF) {
    mCounter = 0x01;
  } else {
    mCounter += 1;
  }
  
  // Keep the watchdog happy
  crypto_feed_watchdog();

  // Generates a random IV
  RNG.rand(outgoingIV, sizeof(outgoingIV));
  
  // Initialize the cipher
  cipher->clear();
  cipher->setKey(my_key, cipher->keySize());
  cipher->setIV(outgoingIV, sizeof(outgoingIV));
  
  // Initialize the buffer
  memset(buffer, 0x00, sizeof(buffer));

  for (pos = 0; pos < datasize; pos += inc) {
    len = datasize - pos;
    if (len > inc) {
      len = inc;
    }
    cipher->encrypt(buffer + pos + off_set, plaintext + pos, len);
  }  
  // Generate encrypt tag
  cipher->computeTag(tag, sizeof(tag));

  // Prepend the IV and Tag to outgoing message
  // Set IV
  for (pos = 0; pos < IV_SIZE; pos++) {
    buffer[pos] = outgoingIV[pos];
  }
  // Set Tag
  for (pos = 0; pos < TAG_SIZE; pos++) {
    buffer[pos + IV_SIZE] = tag[pos];
  }
  
  // Clear the cipher
  delete cipher;
  return buffer;
}


String decryptMessage(byte* message, int mSize) {
  // Declare cipher
  AuthenticatedCipher *cipher = new GCM<AES128>();

  // Indexing variables
  size_t pos, len, inc, datasize = 0;
  
  // Get size of encrypted message
  pos = IV_SIZE + TAG_SIZE;

  datasize = mSize - pos;
  
  inc = datasize;

  // Get Incoming IV value
  for (pos = 0; pos < IV_SIZE; pos++) {
    incomingIV[pos] = message[pos];
  }
  // Get the incoming message tag
  for (pos = 0; pos < TAG_SIZE; pos++) {
    checktag[pos] = message[pos + IV_SIZE];
  }
  // Get the encrypted message
  for (pos = 0; pos < datasize; pos++) {
    ciphertext[pos] = message[pos + IV_SIZE + TAG_SIZE];
  }
  
  // Keep the watchdog happy
  crypto_feed_watchdog();
  
  // Initialize the cipher
  cipher->clear();
  cipher->setKey(my_key, cipher->keySize());
  cipher->setIV(incomingIV, sizeof(incomingIV));

  // Initialize the buffer
  memset(buffer, 0x00, sizeof(buffer));

  // Decrypt the message to the buffer
  for (pos = 0; pos < datasize; pos += inc) {
    len = datasize - pos;
    if (len > inc) {
      len = inc;
    }
    cipher->decrypt(buffer + pos, ciphertext + pos, len);
  }

  // Validate the message tag
  if (!cipher->checkTag(checktag, sizeof(checktag))) {
    notify_select = 5;
    notify_flag = true;
    return fail;
  }

  // Parse the decrypted message
  rCounter = buffer[0];
  pos = 1;
  while (buffer[pos] != 0x00) {
    plaintext[pos - 1] = buffer[pos];
    pos += 1;
  }
  plaintext[pos - 1] = 0x00;

  // Clear cipher object
  delete cipher;
  return (char*)plaintext;
}


int getBtSize(char* buf) {
  return ((buf[0] - 48) * 100) + ((buf[1] - 48) * 10) + (buf[2] - 48);
}
