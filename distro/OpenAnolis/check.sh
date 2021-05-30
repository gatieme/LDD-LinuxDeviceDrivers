#!/bin/bash

rm -rf changelog.txt
branch=anolis_linux-next_4.19
start_task=""
for commit in $(git rev-list $branch)
do
	title=`git show $commit | head -n 20 | grep -E "task #|to #|fix #" | head -n 1`
	task=`git show $commit | head -n 20 | grep -E "task #|to #|fix #" | awk -F'#' '{ print $2 }' | head -n 1`
	#if [ $start_task == "" -a ! -z $task ]; then
	#echo "{$start_task}, {$title}"
	if [ "$start_task" == "" ] && [ ! -z "$task" ]
	then
		start_title=$title
		start_task=$task
		start_commit=$commit
		echo "$task"
	elif [ "$start_task" != "" ] && [ -z "$task" ]
	then
		end_commit=$commit
		echo "=======" | tee -a changelog.txt
		echo "$start_title" | tee -a changelog.txt
		echo "start_commit : $start_commit" | tee -a changelog.txt
		echo "end_commit   : $end_commit" | tee -a changelog.txt
		git log --oneline $start_commit...$end_commit >> changelog.txt
		echo "=======" | tee -a changelog.txt
		start_task=""
		start_title=""
		#exit 0
	elif [ "$start_task" != "" ] && [ "$task" != "$start_task" ]
	then
		end_commit=$commit

		echo "=======" | tee -a changelog.txt
		echo "$start_title" | tee -a changelog.txt
		echo "start_commit : $start_commit" | tee -a changelog.txt
		echo "end_commit   : $end_commit" | tee -a changelog.txt
		git log --oneline $start_commit...$end_commit >> changelog.txt
		echo "=======" | tee -a changelog.txt
		start_task=$task
		start_commit=$commit
		start_title=$title
		#exit 0
	fi
done
