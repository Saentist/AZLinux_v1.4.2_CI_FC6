BUILD_DIR := $(shell pwd)/v4l
TMP ?= /tmp
REPO_PULL := http://linuxtv.org/hg/v4l-dvb

ifeq ($(EDITOR),)
  ifeq ($(VISUAL),)
	EDITOR := vi
  else
	EDITOR := $(VISUAL)
  endif
endif

all:

install:
	$(MAKE) -C $(BUILD_DIR) install

%::
	$(MAKE) -C $(BUILD_DIR) $(MAKECMDGOALS)

commit cvscommit hgcommit change changes changelog:: whitespace
	cd $(BUILD_DIR); scripts/cardlist; cd ..
	v4l/scripts/prep_commit_msg.pl $(TMP)/v4l_hg_whitespace > \
		$(TMP)/v4l_hg_commit.msg
	$(EDITOR) $(TMP)/v4l_hg_commit.msg
	grep -v '^#' $(TMP)/v4l_hg_commit.msg | hg commit -l -
	@echo "*** PLEASE CHECK IF LOG IS OK:"
	@hg log -v -r -1
	@echo "*** If not ok, do hg undo and make commit again"

pull update v4l-update::
	@echo "Pulling changes from master repository $(REPO_PULL)"
	-hg pull -u $(REPO_PULL)

push::
	@echo "Pushing changes to master repository"
	-hg push

whitespace:
	@echo "Cleaning bad whitespaces"
	@v4l/scripts/strip-trailing-whitespaces.sh fast | \
		tee $(TMP)/v4l_hg_whitespace | patch -p0
