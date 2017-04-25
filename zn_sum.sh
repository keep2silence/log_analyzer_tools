#!/bin/bash

for day in 20*
do
	cd $day
	for file in `echo *.csv`
	do
		#echo $file
		#20170119_testcase.csv_7_1.6_1.csv
		# ARG=`echo $file | sed 's/2*_testcase.csv_//g' | sed 's/.csv//'`
		#echo $file
		ARG=`echo $file | sed 's/.*_testcase.csv_//g'| sed 's/.csv//'`
		ARG2=`cat $file`
		echo "$day,$ARG,$ARG2"
	done
	cd - > /dev/null
done
