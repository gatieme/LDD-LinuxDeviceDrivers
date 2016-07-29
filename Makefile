all:github



COMMIT="更新了CFS调度器类的学习实例和博客信息..."

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


