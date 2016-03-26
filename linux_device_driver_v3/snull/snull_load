#!/bin/sh

export PATH=/sbin:/bin

# Use a pathname, as new modutils don't look in the current dir by default
insmod ./snull.ko $*
ifconfig sn0 local0
ifconfig sn1 local1
