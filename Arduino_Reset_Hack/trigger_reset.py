"""
	Simple script for reseting Microbit via Arduino Uno
"""


import serial
import time

arduino = serial.Serial(port='/dev/ttyACM1', baudrate=115200, timeout=0.25)

def trig_reset():
	global arduino
	arduino.write("RESET ".encode())
	time.sleep(1.0)
	print(arduino.readline().decode('utf-8').rstrip())

if __name__ == "__main__":
	time.sleep(2)
	trig_reset()
	arduino.close()
