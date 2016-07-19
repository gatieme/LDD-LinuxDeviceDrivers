all:github



COMMIT="完善了thread_info的学习代码--print_thread_info驱动"

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


