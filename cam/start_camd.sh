#!/bin/bash
#Helper script to start RaspiFastCamD

#We would like to write this to /var/run, but this need root privileges...
pid_file=/tmp/raspifastcamd.pid

if [ -e $pid_file ]; then
	echo "Error: The pid file $pid_file exists, looks like raspifastcamd is already running."
	echo "If this is not the case delete the file."
	exit 1
fi

#output_file=${1-~/tmp_%d.jpg}
output_file=/home/pi/ILI9341/1.jpg
echo "Output will be written to $output_file"

#This will make pictures of 200x200px feel free to change.
/home/pi/ILI9341/cam/raspifastcamd -w 320 -h 240 -o $output_file -q 10 &
pid=$!

echo "Pid of raspifastcamd is $pid"

echo $pid > $pid_file

exit 0
