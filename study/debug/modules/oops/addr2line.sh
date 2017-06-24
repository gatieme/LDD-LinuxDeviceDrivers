#!/bin/bash
#
# addr2line.sh -- Convert PC address to source code line, open the file and point to the line
#

ADDR=$1
[ -z "$ADDR" ] && echo -e "Usage: Please specify the PC address\n    $0 PC_ADDR" && exit 1
[ -z "$CROSS_COMPILE" ] && CROSS_COMPILE=arm-none-linux-gnueabi-

ADDR2LINE=${CROSS_COMPILE}addr2line
file_line=`$ADDR2LINE -e vmlinux $ADDR`
if [ "$file_line" == "??:0" ]; then
    echo "ERROR: Can not find the line for $ADDR"
    exit 2
else
    vim -c "set number" -c "set fdm=manual" $(echo $file_line | sed -e "s/:/ +/g")
fi
