pattern="*"
if [ $# -gt 2 ]; then
	echo "usage $0 [pattern]"
elif [ $# -eq 0 ]; then
	stap -l 'kernel.function("*")' | sort 
elif [ $# -eq 1 ]; then
	stap -l 'kernel.function("*")' | sort | grep $1
fi
