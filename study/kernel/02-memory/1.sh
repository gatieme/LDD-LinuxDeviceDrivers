#!/bin/bash

# Simple print cpu topology
# Author: kodango

function get_nr_processor()
{
    grep '^processor' /proc/cpuinfo | wc -l
}

function get_nr_socket()
{
    grep 'physical id' /proc/cpuinfo | awk -F: '{
            print $2 | "sort -un"}' | wc -l
}

function get_nr_siblings()
{
    grep 'siblings' /proc/cpuinfo | awk -F: '{
            print $2 | "sort -un"}'
}

function get_nr_cores_of_socket()
{
    grep 'cpu cores' /proc/cpuinfo | awk -F: '{
            print $2 | "sort -un"}'
}

echo '===== CPU Topology Table ====='
echo

echo '+--------------+---------+-----------+'
echo '| Processor ID | Core ID | Socket ID |'
echo '+--------------+---------+-----------+'

while read line; do
    if [ -z "$line" ]; then
        printf '| %-12s | %-7s | %-9s |\n' $p_id $c_id $s_id
        echo '+--------------+---------+-----------+'
        continue
    fi

    if echo "$line" | grep -q "^processor"; then
        p_id=`echo "$line" | awk -F: '{print $2}' | tr -d ' '`
    fi

    if echo "$line" | grep -q "^core id"; then
        c_id=`echo "$line" | awk -F: '{print $2}' | tr -d ' '`
    fi

    if echo "$line" | grep -q "^physical id"; then
        s_id=`echo "$line" | awk -F: '{print $2}' | tr -d ' '`
    fi
done < /proc/cpuinfo

echo

awk -F: '{
    if ($1 ~ /processor/) {
        gsub(/ /,"",$2);
        p_id=$2;
    } else if ($1 ~ /physical id/){
        gsub(/ /,"",$2);
        s_id=$2;
        arr[s_id]=arr[s_id] " " p_id
    }
}

END{
    for (i in arr)
        printf "Socket %s:%s\n", i, arr[i];
}' /proc/cpuinfo

echo
echo '===== CPU Info Summary ====='
echo

nr_processor=`get_nr_processor`
echo "Logical processors: $nr_processor"

nr_socket=`get_nr_socket`
echo "Physical socket: $nr_socket"

nr_siblings=`get_nr_siblings`
echo "Siblings in one socket: $nr_siblings"

nr_cores=`get_nr_cores_of_socket`
echo "Cores in one socket: $nr_cores"

let nr_cores*=nr_socket
echo "Cores in total: $nr_cores"

if [ "$nr_cores" = "$nr_processor" ]; then
    echo "Hyper-Threading: off"
else
    echo "Hyper-Threading: on"
fi

echo
echo '===== END ====='
