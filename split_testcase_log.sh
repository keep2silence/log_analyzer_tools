#!/bin/bash

if [ $# != 1 ]
then
	echo "split_testcase_log.sh testcase_log.csv"
	exit
fi

for i in `awk -F, '{print $3}' $1 | uniq`; 
do 
	grep $i $1 >> ${i}_testcase.csv; 
done
