all:github



COMMIT="print_task_struct中增加了对进程优先级/调度器类的实验示例..."

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


