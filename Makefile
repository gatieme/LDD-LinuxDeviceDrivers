all:github



GITHUB_COMMIT := $(COMMIT)


ifeq ($(COMMIT), )
	GITHUB_COMMIT="更新了内存管理之内存描述(四)--页帧struct page详解..."
endif


.PHONY	:	github
github:
	git add -A
	git commit -m $(GITHUB_COMMIT)
	git push origin master


