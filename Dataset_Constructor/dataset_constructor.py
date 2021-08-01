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

parse_dir = False
not_btle_err = False
no_pcap_err = False

dataset = {}

# check that the supplied file path exists
def check_file_exists(p_file):
	if not os.path.exists(p_file):
		raise argparse.ArgumentTypeError("{0} does not exist.".format(p_file))
	if not p_file.endswith('.pcap'):
		raise argparse.ArgumentTypeError("{0} is not a pcap file.".format(p_file))
	return p_file


# check that the supplied dir path exists
def check_dir_exists(p_dir):
	global parse_dir
	if not os.path.isdir(p_dir):
		raise argparse.ArgumentTypeError("{0} does not exist.".format(p_dir))
	parse_dir = True
	return p_dir
	
	
# Check that output name is valid
def validate_name(o_name):
	if o_name.endswith(".csv"):
		o_name.replace(".csv", "")
	prohibited = ["/", "\\", "?", "%", "*", ":", "|", "\"", "<", ">", ";", ",", "=", "."]
	for char in prohibited:
		if char in o_name:
			raise argparse.ArgumentTypeError("Output name cannot contain \'{0}\'".format(char))
	o_name = o_name + ".csv"
	return o_name


# verify that pcaps are BTLE protocol
def check_btle_protocol(p_frame):
	if p_frame == "nordic_ble:btle:btcommon":
		return True
	return False


# Build list of pcap files from dir
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
	

# Takes a list of pcap file paths and only returns pcaps using btle protocol
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



# Write the dataset to a csv output file
def write_to_csv():
	global dataset
	

# Setup argparse for recovering command line arguments
parser = argparse.ArgumentParser(description='BTLE Pcap file path.')
group = parser.add_mutually_exclusive_group(required=True)
group.add_argument("--pcap-file", dest="pcap_path", required=False, metavar='\b',
					type=check_file_exists, help="Set path to target PCAP file.")
group.add_argument("--pcap-dir", dest="pcap_dir", required=False, metavar='\b',
					type=check_dir_exists, help="Set path to directory with target PCAP files.")

parser.add_argument("--out-dir", dest="out_path", required=False, metavar='\b',
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
args = parser.parse_args()


print(args.out_name)

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





