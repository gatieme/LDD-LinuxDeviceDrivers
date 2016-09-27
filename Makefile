all:github



GITHUB_COMMIT := $(COMMIT)


ifeq ($(COMMIT), )
	GITHUB_COMMIT="增加了cgroup的学习文档..."
endif


.PHONY	:	github
github:
	git add -A
	git commit -m $(GITHUB_COMMIT)
	git push origin master


