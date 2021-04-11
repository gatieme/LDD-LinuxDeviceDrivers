#!/bin/bash

dmesg -c


for kptr in `seq 0 2`
do
	echo $kptr > /proc/sys/kernel/kptr_restrict
	KPTR=`cat /proc/sys/kernel/kptr_restrict`
	echo "/proc/sys/kernel/kptr_restrict: $KPTR"
	insmod ./kptr_restrict_test.ko
	dmesg -c
	rmmod ./kptr_restrict_test.ko
done
