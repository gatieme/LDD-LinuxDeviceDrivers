all:github



COMMIT="使用cdev_add注册字符设备--books/fengguojin/src/1drivermodel/1-3register_chrdev--http://book.51cto.com/art/201205/337667.htm"

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


