#!/bin/bash

pidof_process=`pidof ex01`

NACTIVE_THREAD=3

while :
do
	tids=`top -H -b -n 1 -p ${pidof_process} | grep -A ${NACTIVE_THREAD} PID | grep -v PID | awk '{print $1}'`
	for i in ${tids}
	do
		echo `date` >> $tid.log
		pstack $tid >> $tid.log
	done
	sleep 0.1
done
