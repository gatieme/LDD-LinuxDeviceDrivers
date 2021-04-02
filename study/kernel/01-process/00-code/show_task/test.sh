#!/bin/bash

if [ $# -eq 1 ]; then
	PID=$1
else
	PID=$$
fi


insmod show_task.ko

echo $PID >  /proc/sched_show_task

cat /proc/sched_show_task

rmmod show_task.ko
