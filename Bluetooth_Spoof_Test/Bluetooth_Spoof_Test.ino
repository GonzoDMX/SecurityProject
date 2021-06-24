
#include "BluetoothSerial.h"

uint8_t newMac[] = { 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF };


// SETUP Bluetooth ---------- //
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;


void setup() {
  Serial.begin(115200);
  delay(500);
  esp_base_mac_addr_set(newMac);
  // Start BlueTooth, set device name
  SerialBT.begin("SimpleReceiver");
}

void loop() {
  // put your main code here, to run repeatedly:

}
