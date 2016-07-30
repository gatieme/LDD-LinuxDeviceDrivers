all:github



COMMIT="使用cdev_add注册字符设备..."

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


