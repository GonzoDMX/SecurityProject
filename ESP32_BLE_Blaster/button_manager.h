/*
 *    Created by: Andrew O'Shei
 * 
 *    Manages button presses with Debounce
 * 
 */

#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>


// BUTON DECLARATIONS ----------------- //
extern const int BUTTON_COUNT;

extern const int BUTTONS[5];

extern int BUTTON_NEW[5];

extern int BUTTON_OLD[5];

extern unsigned long BUTTON_DELAY[5];

// Set button debounce delay
extern const long delayDebounce;
// ------------------------------------ //

void buttonSetup();

bool evtButton(int newEtat, int* oldEtat, unsigned long* bounce);

int updateButtons();

#endif
