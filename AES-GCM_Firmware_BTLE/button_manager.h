/*
 *    Created by: Andrew O'Shei
 * 
 *    Button Manager
 * 
 */


 #ifndef BUTTON_MANAGER_H
 #define BUTTON_MANAGER_H

#include <Arduino.h>

void setupLedButtons();

void updateButtons();

void checkButton(int button, int *etat, long *bounce, int *led_etat, int led_pin);

void checkBlack();

#endif
