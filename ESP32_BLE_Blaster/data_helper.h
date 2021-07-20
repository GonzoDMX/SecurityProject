/*
 *    Created by: Andrew O'Shei
 * 
 *    Various functions to help parse and process data
 * 
 */

#ifndef DATA_HELPER_H
#define DATA_HELPER_H

#include <Arduino.h>

// Returns the binary segment of a value
unsigned getBinarySegment(unsigned value, unsigned lsb, unsigned msb);

// Convert a 2-byte hex string to bytes
uint8_t convertToHexByte(char msb, char lsb);


// Returns char array
char* getCharArray(String text);

char* getIntAsCharArray(int val);

#endif
