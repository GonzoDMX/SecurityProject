/*
 *    Created by: Andrew O'Shei
 * 
 *    Various functions to help parse and process data
 * 
 */
 
#include "data_helper.h"

char trans_buffer[512];

/*  
 *   Returns the binary segment of a value
 *   The returned value is offset such that lsb is the least significant bit
 */
unsigned getBinarySegment(unsigned value, unsigned lsb, unsigned msb) {
  unsigned temp = 0;
  for (unsigned i=lsb; i<=msb; i++) {
    temp |= 1 << i;
  }
  unsigned ret = (temp & value) >> lsb;
  return ret;
}



// Convert a 2-byte hex string to bytes
uint8_t convertToHexByte(char msb, char lsb) {
  int hi = msb - 48;
  int low = lsb - 48;
  if (hi > 9) {
     hi = hi - 39;
  }
  if (low > 9) {
    low = low - 39;
  }
  return ((hi * 16) + low);
}


char* getCharArray(String text) {
  memset(trans_buffer, 0x00, sizeof(trans_buffer));
  for(int i = 0; i < text.length(); i++) {
    trans_buffer[i] = (char)text[i];
  }
  return trans_buffer;
}

char buff[5];
char* getIntAsCharArray(int val) {
    if (val > 9999) {
        char maxVal[6] = { '+', '9', '9', '9', '9' };
        for (int i = 0; i < 6; i++) {
            buff[i] = maxVal[i];
        }
        return buff;
    } else {
        sprintf(buff, "%d", val);
        return buff;
    }
}
