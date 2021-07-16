# Embedded Security Project
This is a university project researching security for embedded systems. I take a look in particular at using Bluetooth Classic and Bluetooth Low Energy as a means to exploit embedded systems. Testing is based around an Android messaging application that communicates with an ESP32. The Android application was modified from my ColorTooth project in order to provide a platform for testing different cryptographic algorithms. The current version is based around Bluetooth Low Energy and employs an AES-GCM hybrid block cipher algorithm for encrypting the messages passed between the Android app and the ESP32.

## Included:
* Messenger BLE - Android app for sending and receiving encrypted messages over Bluetooth LE.
	1. Messages are encrypted using AES 128-bit / GCM Encryption.
	1. Bluetooth LE and AES-128 bit were chosen to reflect a resource constrained system.
	1. App allows scanning for BLE devices and returns a list of devices, MAC addresses and RSSI signal strength.
* ESP32 Pentester - An attempt at building a Bluetooth Classic pentesting tool out of an ESP32
* 

