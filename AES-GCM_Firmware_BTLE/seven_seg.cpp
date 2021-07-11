/*
 *    Created by: Andrew O'Shei
 * 
 *    Seven Segment Display Manager
 * 
 */

#include "seven_seg.h"

 // SETUP GPIO PINS ---------- //
// Set 7-Seg Display Pins
#define SEG_DIN     19
#define SEG_CS      5
#define SEG_CLK     18

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

// Display Message received counter
unsigned long messageCounter = 0;

// Setup Pins and Init Seven Segment Display
void setupSevSeg() {
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
  zeroDisplay();
}


// Writes data to the 7 segment display
void setDisplay(byte address, byte value) {
   digitalWrite(SEG_CS, LOW);
   shiftOut(SEG_DIN, SEG_CLK, MSBFIRST, address);
   shiftOut(SEG_DIN, SEG_CLK, MSBFIRST, value);
   digitalWrite(SEG_CS, HIGH);
}


// Display loading effect
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

// Set display to Zeros
void zeroDisplay() {
  for (int i = 1; i < 9; i++) {
    setDisplay(i, disp_char[0]);
  }
}

// Message received increment counter
void incrementCount() {
  messageCounter += 1;
  if (messageCounter > 99999999) {
    messageCounter = 0;
  }
}

// Write counter value to the display
void countDisplay() {
  int k = 10000000;
  for (int i = 8; i > 0; i--) {
    setDisplay(i, disp_char[messageCounter / k % 10]);
    k = k / 10;
  }
}

// Write Status Messages to 7-Segment Display
void setMessage(byte* message) {
  int digit = 8;
  for(int i = 0; i < 8; i++) {
    setDisplay(digit - i, message[i]);
  }
}

void updateDisp(int i) {
    switch (i) {
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
}
