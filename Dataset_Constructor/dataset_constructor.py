#!/usr/bin/python3

"""

	Script for parsing BTLE Pcaps
	This script creates a dataset output as a .csv file
	It is used for training a neural network

"""

import pyshark
import argparse
import os
import csv
import string

parse_dir = False
not_btle_err = False
no_pcap_err = False


# CSV file write path
output_path = ""

start_time = 0.0
sample_size = 0.25


""" ----- DATASET SAMPLE DATA POINTS -----"""
# List of dictionary objects for writing dataset to csv file
dataset = []
dataset_header = [	"DATA_SIZE",
					"ADV_IND", 
					"ADV_DIRECT_IND", 
					"ADV_NONCONN_IND", 
					"ADV_SCAN_IND",
					"SCAN_REQ",
					"SCAN_RSP",
					"CONNECT_REQ", 
					"BAD_CRC", 
					"MALFORMED_PACKETS",
					"ATTACK" ]

# Cound total bytes sent over sample period
data_size = 0

""" 
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
# Count number of each PDU type received in sample
adv_ind = 0
adv_dir = 0
adv_non = 0
adv_scn = 0
scn_req = 0
scn_rsp = 0
con_req = 0

# Count number of malformed packets received in sample
malformed = 0

# Count total number of bad CRC checks over sample period
crc_bad = 0


# Flag a sample that contains an attack
#	0 = No Attack
#	1 = Jamming
#	2 = Hijacking
#	3 = Mitm
attack = 0

""" --------------------------------------"""


''' check that the target file path exists '''
def check_file_exists(p_file):
	if not os.path.exists(p_file):
		raise argparse.ArgumentTypeError("{0} does not exist.".format(p_file))
	if not p_file.endswith('.pcap'):
		raise argparse.ArgumentTypeError("{0} is not a pcap file.".format(p_file))
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
	global sample_size
	if size:
		sample_size = float(size)
	print("Sample size set to: " + str(sample_size) + " seconds")
	return float(size)
	

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
	global dataset_header
	try:
		with open(output_path, 'w') as csv_file:
			writer = csv.DictWriter(csv_file, fieldnames = dataset_header)
			writer.writeheader()
			for data in dataset:
				writer.writerow(data)
	except Exception as e:
		print(e)

#TODO
""" $$$$$$$$$$$$$$$$ TODO $$$$$$$$$$$$$$$$$$$$$$ """
# Parse PCAP and build sample frame here
def build_sample_frame(cap):
	global crc_bad
	global data_size
	global malformed
	
	pdu_type = int(cap[index][1].advertising_header_pdu_type)
	
	if pdu_type == 3:
		print("Scanning")
		bd_addr = cap[index][0].scanning_address
	elif pdu_type == 5:
		print("Connecting")
	else:
		print("Advertising")
		bd_addr = cap[index][0].advertising_address
	
	# Check if CRC is bad -> Get total number of bad CRCs per sample
	if cap[index][0].crc_bad == "CRC is bad":
		crc_bad += 1
		
	# Get total packet size -> Get total data amount sent over sample period
	p_size = int(cap[index][1].length)
	data_size += p_size
""" $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ """


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

parser.add_argument("--attack", action='store_true', required=False,
					dest="mark_att",
					help="Files including \'attack\' in name " + 
					"will be flagged in output file.")

parser.add_argument("--mac-filter", nargs="+", default=[], required=False,
					dest="mac_filter", metavar='BD_ADDR', type=str,
					help="Takes one or more BTLE BD_ADDRs, " + 
					"output data will only include packets with BD_ADDRs in list.")

parser.add_argument("--sample-size", dest="sample_size", required=False,
					metavar='\b', type=set_sample_size,
					help="Set packet sample size in seconds.")
args = parser.parse_args()


# Set output file name and path
set_output_path(args.out_name, args.out_dir)

		
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
		
	else:
		# TODO Build handling for parsing single files
		print("Loading PCAP File: " + args.pcap_path)
		# Use pyshark to read the selected PCAP file
		cap = pyshark.FileCapture(args.pcap_path)
		# Verify the pcaps are BTLE
		if check_btle_protocol(cap[0].frame_info.protocols):
			# Get number of packets in pcap, they must be loaded first
			cap.load_packets()
			print("Number of packets in pcap:" + str(len(cap)))
			
			# Returns delta time as string
			# print(cap[1][0].delta_time)

			# Returns delta time as int
			# print(int(cap[1][0].delta_time))

			# Print Packet[1] Layer[0]
			print(cap[1][0])

			# Print Packet[1] Layer[1]
			print(cap[1][1])
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

except Exception as e:
	print(e)





