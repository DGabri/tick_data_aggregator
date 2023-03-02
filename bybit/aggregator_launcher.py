#!/usr/bin/python3
import subprocess
import sys
import os

if (len(sys.argv) < 0):
    print("Wrong arguments, please choose the desired resample frequency in seconds")
    sys.exit(1)

# get the current resample frequency to pass to the c aggregator script
resample_frequency = sys.argv[1]
files = os.listdir()

for input_file in files:
    # get all csv files to pass them to the c aggregator

    if (input_file.endswith(".csv")):
        # output filename
        coin = input_file.split(".")[0]
        output_file = str(coin) + "_" + str(resample_frequency) + ".csv"
        subprocess.run(["./main", input_file, output_file, resample_frequency])