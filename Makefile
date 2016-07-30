all:github



COMMIT="测试的proc_test的驱动proc_final包括只读proc_entry, 只写proc_entry和可读写proc_entry..."

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


