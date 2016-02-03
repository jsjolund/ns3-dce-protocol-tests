#!/bin/bash
echo "*******************************************************************************"
echo "Converting $file.pcap to text format...."
tshark -V -r $file.pcap > $file.txt


