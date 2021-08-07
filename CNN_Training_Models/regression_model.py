"""
	Tensorflow tutorial
	How to load csv datasets
"""


import pandas as pd
import numpy as np

# Make numpy values easier to read
np.set_printoptions(precision=3, suppress=True)

import tensorflow as tf
from tensorflow.keras import layers
from tensorflow.keras.layers.experimental import preprocessing

dataset_file = "/home/andrew/Git_Repos/Security_Project/Dataset_Constructor/out.csv"

# Create a Pandas Data Frame from the dataset
pcap_train = pd.read_csv(dataset_file,
						names=[	"NUMBER",			#
								"PACKET_COUNT", 	#
								"ADDR_COUNT",		#
								"CHAN_COUNT",		#
								"DATA_SIZE",		#
								"AVG_SIZE",			#
								"MIN_SIZE",			#
								"MAX_SIZE",			#
								"ADV_IND", 			# 
								"ADV_DIRECT_IND",	#
								"ADV_NONCONN_IND", 	#
								"ADV_SCAN_IND",		#
								"SCAN_REQ",			#
								"SCAN_RSP",			#
								"CONNECT_REQ",		#
								"AVG_RSSI", 		#
								"MIN_RSSI",			#
								"MAX_RSSI",			#
								"CRC_BAD", 			#
								"CRC_WEIGHT",		#
								"ATTACK"			#
								],
						skipinitialspace = True, skiprows = 1, engine = "python")
pcap_train.head()

# We want to train this model to predict attacks, so separate for training
pcap_features = pcap_train.copy()
pcap_labels = pcap_features.pop('ATTACK')

# Do this to treat all features identically
pcap_features = np.array(pcap_features)
pcap_features

# Normalize the data
normalize = preprocessing.Normalization()
normalize.adapt(pcap_features)

# Make a regression model to predict attack since there is only one tensor Sequential is sufficient
pcap_model = tf.keras.Sequential([normalize, layers.Dense(64), layers.Dense(1)])

pcap_model.compile(loss = tf.losses.MeanSquaredError(),
					optimizer = tf.optimizers.Adam(),
					metrics=['accuracy'])
					
# To train the model pass the features and labels to Model.fit
pcap_model.fit(pcap_features, pcap_labels, epochs=10)





