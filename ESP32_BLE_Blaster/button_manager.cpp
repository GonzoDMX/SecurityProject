/*
 *    Created by: Andrew O'Shei
 * 
 *    Manages button presses with Debounce
 * 
 */


#include "button_manager.h"


// BUTON DECLARATIONS ----------------- //
/*
*
*   Button Up = 25
*   Button Down = 34
*   Button Left = 33
*   Button Right = 32
*   Button Middle = 35
*
*/
const int BUTTON_COUNT = 5;

const int BUTTONS[5] = { 25, 34, 33, 32, 35 };

int BUTTON_NEW[5] = { 0, 0, 0, 0, 0 };

int BUTTON_OLD[5] = { 0, 0, 0, 0, 0 };

unsigned long BUTTON_DELAY[5] = { 0, 0, 0, 0, 0 };

// Set button debounce delay
const long delayDebounce = 100;
// ------------------------------------ //

// Setup button pins
void buttonSetup() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(BUTTONS[i], INPUT);
  }
}

// Check if button is pressed and apply a debounce
bool evtButton(int newEtat, int* oldEtat, unsigned long* bounce) {
  if ((millis() - *bounce) > delayDebounce) {
    if (newEtat == HIGH && *oldEtat == 0) {
      *oldEtat = 1;
      *bounce = millis();
      return true;
    }
    else if (newEtat == LOW && *oldEtat == 1) {
      *oldEtat = 0;
      *bounce = millis();
      return false;
    }
  }
  return false;
}

// Read button states
int updateButtons() {
  // Check each button and see if pressed
  for (int i = 0; i < BUTTON_COUNT; i++) {
    BUTTON_NEW[i] = digitalRead(BUTTONS[i]);
    if (evtButton(BUTTON_NEW[i], &BUTTON_OLD[i], &BUTTON_DELAY[i])) {
      return i + 1;
    }
  }
  return 0;
}
