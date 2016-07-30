all:github



COMMIT="完善了thread_info的学习..."

.PHONY	:	github
github:
	git add -A
	git commit -m $(COMMIT)
	git push origin master


