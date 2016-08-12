all:github



GITHUB_COMMIT := $(COMMIT)


ifeq ($(COMMIT), )
	GITHUB_COMMIT="..."
endif


.PHONY	:	github
github:
	git add -A
	git commit -m $(GITHUB_COMMIT)
	git push origin master


