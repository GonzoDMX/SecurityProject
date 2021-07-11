/*
 *    Created by: Andrew O'Shei
 * 
 *    Button Manager
 * 
 */


#include "button_manager.h"


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

// Setup Pins for buttons and leds
void setupLedButtons() {
  // Setup LED pin mode
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GRN, OUTPUT);
  pinMode(LED_BLU, OUTPUT);

  // Set all LEDs to off
  digitalWrite(LED_RED, 0);
  digitalWrite(LED_GRN, 0);
  digitalWrite(LED_BLU, 0);

  // Setup push button pin mode
  pinMode(BUTTON_RED, INPUT);
  pinMode(BUTTON_GRN, INPUT);
  pinMode(BUTTON_BLU, INPUT);
  pinMode(BUTTON_BLK, INPUT);
}

void updateButtons() {
  // CHECK FOR BUTTON PRESSES --------- //
  // Read current button states
  B_RED = digitalRead(BUTTON_RED);
  B_GRN = digitalRead(BUTTON_GRN);
  B_BLU = digitalRead(BUTTON_BLU);
  B_BLK = digitalRead(BUTTON_BLK);

  // Check if buttons are pressed
  checkButton(B_RED, &S_RED, &D_RED, &L_RED, LED_RED);
  checkButton(B_GRN, &S_GRN, &D_GRN, &L_GRN, LED_GRN);
  checkButton(B_BLU, &S_BLU, &D_BLU, &L_BLU, LED_BLU);
  checkBlack();
  // ---------------------------------- //
}

// Toggle LED Buttons
void checkButton(int button, int *etat, long *bounce, int *led_etat, int led_pin) {
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
void checkBlack() {
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
