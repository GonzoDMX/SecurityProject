#!/usr/bin/python3

"""

	Script for parsing BTLE Pcaps
	This script creates a dataset output as a .csv file
	It is used for training a neural network
	
	
	----- List of PDU Types -----

	Advertising PDUs:
			ADV_IND			(0x0)	Connectable Undirected Advertising
			ADV_DIRECT_IND 	(0x1)	Connectable Directed Advertising
			ADV_NONCONN_IND (0x2)	Non-Connectable Undirected Advertising
			ADV_SCAN_IND	(0x6)	Scannable Undirected Advertising
	
	Scanning PDUs:
			SCAN_REQ		(0x3)	Scan request
			SCAN_RSP		(0x4)	Scan response
	
	Initiating PDUs:
			CONNECT_REQ		(0x5)		Connection request

"""

import pyshark
import argparse
import os
import csv
import string


# Global flag for setting batch processing
parse_dir = False

# Global flags for seting appropriate Error Messages
not_btle_err = False
no_pcap_err = False

# Global variable for holding the Dataset Class
dataset = None

# Address Filter list
filter_list = []

# CSV file write path
output_path = ""

# Used to parse attack times and mark affected packets
attack_list = []

# Flag a sample that contains an attack
#		0 = No Attack
#		1 = Jamming
#		2 = Hijacking
#		3 = Mitm
attack = 0

# Holds last time of last packet in pcap file
roll_over = 0.0

""" ----- DATASET SAMPLE DATA POINTS -----"""
class PCAP_Dataset:
	def __init__(self, sample_size):
		# Store start time, used when analyzing multiple pcap files
		self.start = 0.0

		# Set size of sample in seconds
		if float(sample_size) > 0.0:
			self.size = sample_size
		else:
			self.size = 0.25

		# Set dictionary keys and 
		self.header = [	"NUMBER",		# Counts to sample number starting from 0
						"PACKET_COUNT", #
						"ADDR_COUNT",	# cap[i][1].scanning_address \\ cap[i][1].advertising_address
						"CHAN_COUNT",	# cap[i][0].channel
						"DATA_SIZE",	# cap[i][1].length
						"AVG_SIZE",		#
						"MIN_SIZE",		#
						"MAX_SIZE",		#
						"ADV_IND", 		# cap[i][1].advertising_header_pdu_type 
						"ADV_DIRECT_IND",	# "
						"ADV_NONCONN_IND", 	# "
						"ADV_SCAN_IND",		# "
						"SCAN_REQ",			# "
						"SCAN_RSP",			# "
						"CONNECT_REQ",		# "
						"AVG_RSSI", 		# cap[i][0].rssi
						"MIN_RSSI",			#
						"MAX_RSSI",			#
						"CRC_BAD", 			# cap[i][0].crc_bad
						"CRC_WEIGHT",		# 1 if more likely to be bad else 0
						"ATTACK" ]			#
		# Declare index for accessing current sample
		self.i = 0
		
		# Declare counter for CRC Checks
		self.crc_count = 0
		
		# Dictionary template for building a sample
		self.template = {}
		
		# List of dictionary objects for writing dataset to csv file
		self.sample = []

		# Initialize sample
		for item in self.header:
			self.template[item] = 0
		self.sample.append(self.template.copy())

		# List contains all unique addresses in a sample
		# The list is used to count number of unique addresses
		self.addr_list = []

		# List contains all unique channels in a sample
		self.chan_list = []

		# Gets all packet sizes for analysis
		self.size_list = []

		# Average the RSSI value of captured packets
		self.rssi_list = []

	# Create a new empty sample
	def add_sample(self):
		self.i += 1
		self.sample.append(self.template.copy())
		self.sample[self.i]["NUMBER"] = self.i
		self.crc_count = 0
		self.addr_list = []
		self.chan_list = []
		self.size_list = []
		self.rssi_list = []
		
	# Add packet to count
	def increment_packets(self):
		self.sample[self.i]["PACKET_COUNT"] += 1
		
	# Remove packet from count, happens if packet hits a filter
	def decrement_packets(self):
		self.sample[self.i]["PACKET_COUNT"] -= 1
		
	# If address is new, add to list and increment count
	def add_address(self, addr):
		if addr and addr not in self.addr_list:
			self.addr_list.append(addr)
			self.sample[self.i]["ADDR_COUNT"] = len(self.addr_list)
		
	# If channel is new, add to list and increment count
	def add_channel(self, chan):
		if chan not in self.chan_list:
			self.chan_list.append(chan)
			self.sample[self.i]["CHAN_COUNT"] = len(self.chan_list)
		
	# Parse data list
	def set_data_size(self, size):
		self.sample[self.i]["DATA_SIZE"] += size
		self.size_list.append(size)
		self.size_list.sort()
		self.sample[self.i]["MIN_SIZE"] = self.size_list[0]
		self.sample[self.i]["MAX_SIZE"] = self.size_list[-1]
		self.sample[self.i]["AVG_SIZE"] = round(sum(s for s in self.size_list) / len(self.size_list), 2)
		
	# Increment the corresponding PDU Type
	def set_pdu(self, pdu):
		if not pdu:
			self.sample[self.i]["ADV_IND"] += 1
		elif pdu == 1:
			self.sample[self.i]["ADV_DIRECT_IND"] += 1
		elif pdu == 2:
			self.sample[self.i]["ADV_NONCONN_IND"] += 1
		elif pdu == 6:
			self.sample[self.i]["ADV_SCAN_IND"] += 1
		elif pdu == 3:
			self.sample[self.i]["SCAN_REQ"] += 1
		elif pdu == 4:
			self.sample[self.i]["SCAN_RSP"] += 1
		elif pdu == 5:
			self.sample[self.i]["CONNECT_REQ"] += 1
			
	# Update the RSSI average
	def set_rssi(self, rssi):
		self.rssi_list.append(rssi)
		self.sample[self.i]["AVG_RSSI"] = round(sum(r for r in self.rssi_list) / len(self.rssi_list), 2)
		self.rssi_list.sort()
		self.sample[self.i]["MIN_RSSI"] = self.rssi_list[0]
		self.sample[self.i]["MAX_RSSI"] = self.rssi_list[-1]

	# Update CRC Check values
	def set_crc_weight(self, crc):
		self.sample[self.i]["CRC_BAD"] += crc
		self.crc_count += 1
		self.sample[self.i]["CRC_WEIGHT"] = round(self.sample[self.i]["CRC_BAD"] / self.crc_count, 2)

""" --------------------------------------"""


''' check that the target file path exists '''
def check_file_exists(p_file):
	if not os.path.exists(p_file):
		raise argparse.ArgumentTypeError("{0} does not exist.".format(p_file))
	#if not p_file.endswith('.pcap'):
	#	raise argparse.ArgumentTypeError("{0} is not a pcap file.".format(p_file))
	return p_file


''' check that the target dir path exists '''
def check_dir_exists(p_dir):
	global parse_dir
	if not os.path.isdir(p_dir):
		raise argparse.ArgumentTypeError("{0} does not exist.".format(p_dir))
	parse_dir = True
	return p_dir
	
	
''' Check that output name is valid '''
def validate_name(o_name):
	if o_name.endswith(".csv"):
		o_name = o_name.replace(".csv", "")
	prohibited = ["/", "\\", "?", "%", "*", ":", "|", "\"", "<", ">", ";", ",", "=", "."]
	for char in prohibited:
		if char in o_name:
			raise argparse.ArgumentTypeError("Output name cannot contain \'{0}\'".format(char))
	o_name = o_name + ".csv"
	return o_name


''' Set the output path '''
def set_output_path(name, directory):
	global output_path
	# Set name if used set name, else use default
	output_name = "out.csv"
	if name:
		output_name = name
	# Set the output directory path
	if directory:
		if directory.endswith("/"):
			output_path = directory + output_name
		else:
			output_path = directory + "/" + output_name
	else:
		output_path = os.getcwd() + "/" + output_name
	# If ouutput path already exists increment name
	count = 1
	while os.path.exists(output_path):
		output_path = output_path.replace(".csv", "")
		if count > 1:
			output_path = output_path.rstrip(string.digits)
			output_path = output_path[:-1]
		output_path = output_path + "_" + str(count) + ".csv"
		count += 1
	print("Output path set: " + output_path)
	
	
''' Set the sample size of dataset '''
def set_sample_size(size):
	try:
		return float(size)
	except ValueError:
		print("Error: Provided sample size is invalid")
		return 0.25
	

''' verify that pcaps are BTLE protocol '''
def check_btle_protocol(p_frame):
	if p_frame == "nordic_ble:btle:btcommon":
		return True
	return False


''' Build a list of pcap files from dir '''
def collect_pcap_files(p_dir):
	global no_pcap_err
	p_files = list()
	for entry in os.scandir(p_dir):
		if entry.is_file and entry.path.endswith(".pcap"):
			p_files.append(entry.path)
	if not p_files:
		no_pcap_err = True
		raise TypeError
	p_files.sort()
	return p_files
	

''' Takes a list of pcap file paths and only returns pcaps using btle protocol '''
def check_dir_btle(p_files):
	global no_pcap_err
	for file in p_files:
		cap = pyshark.FileCapture(file)
		if not check_btle_protocol(cap[0].frame_info.protocols):
			print(file + " --> Is not btle protocol and will not be processed.")
			p_files.remove(file)
		cap.close()
	if not p_files:
		no_pcap_err = True
		raise IndexError
	return p_files



''' Write the dataset to a csv output file '''
def write_to_csv():
	global dataset
	try:
		print("Writing to CSV file...")
		with open(output_path, 'w') as csv_file:
			writer = csv.DictWriter(csv_file, fieldnames = dataset.header)
			writer.writeheader()
			for data in dataset.sample:
				writer.writerow(data)
	except Exception as e:
		print("write_to_csv error")
		print(e)


''' Gather information and parse from an individual packet'''
def analyse_packet(cap, i):
	global dataset
	global attack
	dataset.increment_packets()
	for ind in range(8):
		try:
			if ind == 0:
				# Collect Scanning address
				dataset.add_address(cap[i][1].scanning_address)

			elif ind == 1:
				# Collect Advertising Address
				dataset.add_address(cap[i][1].advertising_address)
				
			elif ind == 2:
				# Append new channels
				dataset.add_channel(cap[i][0].channel)

			elif ind == 3:
				# Add size of packet to total datasize
				dataset.set_data_size(int(cap[i][1].length))

			elif ind == 4:
				# Add pdu type to corresponding pdu
				dataset.set_pdu(int(cap[i][1].advertising_header_pdu_type))

			elif ind == 5:
				# Add packet rssi to sample rssi average
				dataset.set_rssi(int(cap[i][0].rssi))

			elif ind == 6:
				# Check if CRC is bad -> Get total number of bad CRCs per sample
				if cap[i][0].crc_bad == "CRC is bad":
					dataset.set_crc_weight(1)
		except AttributeError:
			if ind == 6:
				dataset.set_crc_weight(0)
			continue
	dataset.sample[dataset.i]["ATTACK"] = attack


''' Check if packet passes filter '''
def filter_check(cap, i):
	global filter_list
	addrs = []
	try:
		addrs.append(cap[i][1].scanning_address)
	except AttributeError:
		pass
	try:
		addrs.append(cap[i][1].advertising_address)
	except AttributeError:
		pass
	for a in addrs:
		if a in filter_list:
			return True
	return False
	

''' Iterate through all packets in pcap and break into samples '''
def parse_pcap_samples(cap, count):
	global dataset
	global roll_over
	global filter_list
	# Iterate through all packets
	for i in range(0, count):
		p_time = float(cap[i].frame_info.time_relative)
		time = p_time + roll_over - dataset.start
		if time > dataset.size:
			while time > dataset.size * 2:
				dataset.add_sample()
				dataset.start += dataset.size
				time = p_time + roll_over - dataset.start
			dataset.start += dataset.size
			print("Split at packet: " + str(i))
			print("sample" + str(dataset.i + 1))
			dataset.add_sample()
		if not filter_list or filter_check(cap, i):
			analyse_packet(cap, i)
		
	# Set roll_over time for processing multiple packets
	roll_over = p_time + roll_over

		
''' Iterate through all pcap files and break into samples '''
def parse_multi_pcap(p_files):
	global attack
	for p in p_files:
		# Build handling for parsing single files
		print("Loading PCAP File: " + p)
		# Use pyshark to read the selected PCAP file
		cap = pyshark.FileCapture(p)

		# Get number of packets in pcap, they must be loaded first
		cap.load_packets()
		count = len(cap)
		
		print("Number of packets in pcap:" + str(len(cap)))
		
		# Parse pcap file and build samples
		parse_pcap_samples(cap, count)	


# Get attack times for flagging samples
def parse_attack_path(att_path):
	global attack_list
	with open(att_path) as file:
		for row in csv.DictReader(file, skipinitialspace=True):
			tempDict = {}
			tempDict["Time"] = float(row["Time"])
			tempDict["Info"] = row["Info"]
			tempDict["Status"] = int(row["Status"])
			attack_list.append(tempDict)
		return True


''' Main program function, read arg parser and execute in batch or single file mode '''
def main():
	global dataset
	global parse_dir
	global bad_name_err
	global no_pcap_err
	global not_btle_err
	global filter_list
	global attack_path
	# Setup argparse for recovering command line arguments
	parser = argparse.ArgumentParser(description='BTLE Pcap file path.')
	group = parser.add_mutually_exclusive_group(required=True)

	group.add_argument("--pcap-file", dest="pcap_path", required=False, metavar='\b',
						type=check_file_exists, help="Set path to target PCAP file.")

	group.add_argument("--pcap-dir", dest="pcap_dir", required=False, metavar='\b',
						type=check_dir_exists, help="Set path to directory with target PCAP files.")

	parser.add_argument("--out-dir", dest="out_dir", required=False, metavar='\b',
						type=check_dir_exists, help="Set path for the dataset output .CSV file.")

	parser.add_argument("--out-name", dest="out_name", required=False, metavar='\b',
						type=validate_name,
						help="Set name for the dataset output .CSV file (default is: out.csv).")

	parser.add_argument("--attack-log", dest="attack_log", required=False, metavar='\b',
						type=check_file_exists, help="Set attack log for pcap.")

	parser.add_argument("--addr-filter", nargs="+", default=[], required=False,
						dest="addr_filter", metavar='BD_ADDR', type=str,
						help="Takes one or more BTLE BD_ADDRs, " + 
						"output data will only include packets with BD_ADDRs in list.")

	parser.add_argument("--sample-size", dest="sample_size", required=False,
						metavar='\b', type=set_sample_size, default="0.0",
						help="Set packet sample size in seconds.")
	args = parser.parse_args()

	# Send addresses to filter list
	filter_list = args.addr_filter

	# Set output file name and path
	set_output_path(args.out_name, args.out_dir)

	# Declare dataset
	dataset = PCAP_Dataset(args.sample_size)
	
	# Get attack path
	if not parse_attack_path(args.attack_log):
		print("Attack Log Path invalid")
			
	# Parse the supplied pcap file or directory
	try:
		if parse_dir:
			print("Collecting PCAP files in Dir: " + args.pcap_dir)
			# Build a list of the files in pcap_dir
			pcap_files = collect_pcap_files(args.pcap_dir)
			
			# Verify all files use btle protocol
			print("Verifying btle protocol")
			pcap_files = check_dir_btle(pcap_files)

			print("Complete !")		
			print("PCAP files found:")
			for f in pcap_files:
				print(f)
			
			parse_multi_pcap(pcap_files)
			
			write_to_csv()
			
		else:
			# Build handling for parsing single files
			print("Loading PCAP File: " + args.pcap_path)
			# Use pyshark to read the selected PCAP file
			cap = pyshark.FileCapture(args.pcap_path)
			# Verify the pcaps are BTLE
			if check_btle_protocol(cap[0].frame_info.protocols):
				# Get number of packets in pcap, they must be loaded first
				cap.load_packets()
				count = len(cap)
				
				print("Number of packets in pcap:" + str(len(cap)))
				
				# Parse pcap file and build samples
				parse_pcap_samples(cap, count)
				
				write_to_csv()

			else:
				not_btle_err = True
				raise TypeError
				
	except TypeError as e:
		if bad_name_err:
			print("Char: \'" + +"\' cannot be used in output name.")
		elif not_btle_err:
			print("Error: " + args.pcap_path + " is not btle protocol.")
		else:
			print(e)

	except IndexError as e:
		if no_pcap_err:
			print("No PCAP files found in: " + args.pcap_dir)
		else:
			print(e)		



if __name__ == "__main__":
	main()

