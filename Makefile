all:github



COMMIT="list/rbtree遍历进程虚拟地址空间vm_area_struct的学习代码中增加了输出文件映射的代码print_vm_file--驱动print_vmarea"

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


