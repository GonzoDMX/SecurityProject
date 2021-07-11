/*
 *    Created by: Andrew O'Shei
 * 
 *    Seven Segment Display Manager
 * 
 */


 #ifndef SEVEN_SEG_H
 #define SEVEN_SEG_H

#include <Arduino.h>

void setupSevSeg();

void setDisplay(byte address, byte value);

void loadDisplay();

void zeroDisplay();

void incrementCount();

void countDisplay();

void setMessage(byte* message);

void updateDisp(int i);

 #endif
