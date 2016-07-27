all:github



COMMIT="完善了list/rbtree遍历进程虚拟地址空间vm_area_struct的学习代码--驱动print_vmarea"

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


