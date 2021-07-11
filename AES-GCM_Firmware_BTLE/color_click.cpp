/*
 *    Created by: Andrew O'Shei
 * 
 *    Color Click Manager
 * 
 */


 #include "color_click.h"



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


void setupTCS3471() {
  // Start i2C
  Wire.begin();
  delay(250);
  // Check if TCS3471 is connected and functioning
  if(!TCS3471.detect()){
    Serial.println("Error: Color Click not detected!");
    // setMessage(disp_error);
    // while(1);
  }
  
  // Setup TCS3471
  // Range is 2.4ms to 614.4ms, more time means slower but more precise
  TCS3471.setIntegrationTime(350.0);
  // Range is 2.4ms to 7400ms, longer waits mean slower reads but lower power consumption
  TCS3471.setWaitTime(150.0);
  // TCS3471_GAIN_1X, TCS3471_GAIN_4X, TCS3471_GAIN_16X and TCS3471_GAIN_60X
  TCS3471.setGain(TCS3471_GAIN_1X);
  // Enable the TCS3471 Chip
  TCS3471.enable();
  Serial.println("TCS3471 Enabled!");
}
