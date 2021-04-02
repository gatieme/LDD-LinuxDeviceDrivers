#!/bin/bash

insmod show_vma.ko

yes > /dev/null &
pid=$!
echo $pid > /proc/mm_show_vma
cat /proc/mm_show_vma

kill -9 $pid
rmmod show_vma
