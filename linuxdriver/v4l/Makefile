ifeq ($(obj),)
obj = .
endif

#################################################
# Version Check

ifneq ($(KERNELRELEASE),)

-include $(TOPDIR)/Rules.make

else

# take version info from last module build if available
-include $(obj)/.version

ifneq ($(SRCDIR),)
KDIR := $(SRCDIR)
else
ifneq ($(KERNELRELEASE),)
KDIR := /lib/modules/$(KERNELRELEASE)/build
else
KDIR := /lib/modules/$(shell uname -r|perl -ne 'if (/^([0-9]*)\.([0-9])*\.([0-9]*)(.*)$$/) { printf ("%s.%s.%s%s\n",$$1,$$2,$$3,$$4); };')/build

endif
endif

endif

#################################################
# default compilation rule

default:: config-compat.h Makefile.media links .version
	echo srcdir $(SRCDIR)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) $(MYCFLAGS) modules
	./scripts/rmmod.pl check
#################################################
# Object specific rules

-include $(obj)/.myconfig
-include $(obj)/Makefile.media
-include $(obj)/Makefile.ivtv

#################################################
# CFLAGS configuration

ifeq ($(CONFIG_XC3028),m)
 EXTRA_CFLAGS += -DCONFIG_XC3028=1
endif

ifeq ($(CONFIG_DVB_FIRESAT),m)
  EXTRA_CFLAGS += -I$(srctree)/drivers/ieee1394/
endif

EXTRA_CFLAGS += -g

EXTRA_CFLAGS += $(if $(wildcard $(srctree)/.mm), -DMM_KERNEL)

#################################################
# Kernel 2.4/2.6 specific rules

ifneq ($(KERNELRELEASE),)

ifeq ($(VERSION).$(PATCHLEVEL),2.6)
 export-objs	:=
 list-multi	:=
else
include $(obj)/Makefile.kern24

 multi-m	:= $(filter $(list-multi), $(obj-m))
 int-m		:= $(sort $(foreach m, $(multi-m), $($(basename $(m))-objs)))
 export-objs	:= $(filter $(int-m) $(obj-m),$(export-objs))
endif

endif

#################################################
# Directories to be used

snapshot := $(wildcard $(obj)/.snapshot)
ifneq ($(snapshot),)
SNAPSHOT_CFLAGS	:= -DSNAPSHOT='$(shell cat $(snapshot))'
EXTRA_CFLAGS	+= $(SNAPSHOT_CFLAGS)
endif

PWD		:= $(shell pwd)
DEST		:= /lib/modules/$(KERNELRELEASE)/v4l2
KDIR26		:= /lib/modules/$(KERNELRELEASE)/kernel/drivers/media

#################################################
# Compiler fixup rules

HOSTCC:=$(CC)
CC += -I$(obj)

ifeq ($(VERSION).$(PATCHLEVEL),2.6)
 CPPFLAGS := -I$(SUBDIRS)/../linux/include $(CPPFLAGS) -I$(SUBDIRS)/
 MYCFLAGS :=
else
 MYCFLAGS := CFLAGS="-I../linux/include -D__KERNEL__ -I$(KDIR)/include -DEXPORT_SYMTAB"
endif


#################################################
# which files to install?

inst-m		:= $(wildcard *.ko)
ifeq ($(inst-m),)
  inst-m	:= $(obj-m)
endif

v4l_modules := $(shell /sbin/lsmod|cut -d' ' -f1 ) $(patsubst %.ko,%,$(inst-m))

#################################################
# locales seem to cause trouble sometimes.
LC_ALL = POSIX
export LC_ALL


#################################################
# Generic wildcard rules

%.asm: %.o
	objdump -S $< > $@

#################################################
# all file compilation rule

all:: allmodconfig default

#################################################
# installation invocation rules

install:: media-install ivtv-install

remove rminstall:: media-rminstall ivtv-rminstall

#################################################
# Compiling preparation rules

.version::
ifneq ($(KERNELRELEASE),)
	@echo -e VERSION=$(VERSION)\\nPATCHLEVEL:=$(PATCHLEVEL)\\nSUBLEVEL:=$(SUBLEVEL)\\nKERNELRELEASE:=$(KERNELRELEASE) > $(obj)/.version
ifneq ($(SRCDIR),)
	@echo -e SRCDIR=$(SRCDIR)\\n >> $(obj)/.version
endif
else
	@echo "No version yet."
	@uname -r|perl -ne 'if (/^([0-9]*)\.([0-9])*\.([0-9]*)(.*)$$/) { printf ("VERSION=%s\nPATCHLEVEL:=%s\nSUBLEVEL:=%s\nKERNELRELEASE:=%s.%s.%s%s\n",$$1,$$2,$$3,$$1,$$2,$$3,$$4); };' > $(obj)/.version
endif

Makefile.media:: .version
	scripts/make_makefile.pl $(KDIR)

release::
ifneq ($(VER),)
	@echo "Forcing compiling to version $(VER)."
	@echo $(VER)|perl -ne 'if (/^([0-9]*)\.([0-9])*\.([0-9]*)(.*)$$/) { printf ("VERSION=%s\nPATCHLEVEL:=%s\nSUBLEVEL:=%s\nKERNELRELEASE:=%s.%s.%s%s\n",$$1,$$2,$$3,$$1,$$2,$$3,$$4); };' > $(obj)/.version
else
ifneq ($(DIR),)
	@echo "Seeking for a version at $(DIR)/Makefile."
	@perl \
	-e 'open IN,"$(DIR)/Makefile"; ' \
	-e 'while (<IN>) {' \
	-e '	if (/^VERSION\s*=\s*([0-9]+)/){ $$version=$$1; }' \
	-e '	elsif (/^PATCHLEVEL\s*=\s*([0-9]+)/){ $$level=$$1; }' \
	-e '	elsif (/^SUBLEVEL\s*=\s*([0-9]+)/){ $$sublevel=$$1; }' \
	-e '	elsif (/^EXTRAVERSION\s*=\s*([^\s]+)\n/){ $$extra=$$1; }' \
	-e '};' \
	-e 'printf ("VERSION=%s\nPATCHLEVEL:=%s\nSUBLEVEL:=%s\nKERNELRELEASE:=%s.%s.%s%s\n",' \
	-e '	$$version,$$level,$$sublevel,$$version,$$level,$$sublevel,$$extra);' \
	-e 'printf ("SRCDIR:=$(DIR)\n");' > $(obj)/.version
	@cat .version|grep KERNELRELEASE:|sed s,'KERNELRELEASE:=','Forcing compiling to version ',

	@if [ ! -f $(DIR)/scripts/kallsyms ]; then \
		echo "*** Warning: You should configure and build kernel before compiling V4L"; \
	fi
else
	@echo "No version specified. Using `uname -r`."
	@uname -r|perl -ne 'if (/^([0-9]*)\.([0-9])*\.([0-9]*)(.*)$$/) { printf ("VERSION=%s\nPATCHLEVEL:=%s\nSUBLEVEL:=%s\nKERNELRELEASE:=%s.%s.%s%s\n",$$1,$$2,$$3,$$1,$$2,$$3,$$4); };' > $(obj)/.version
endif
endif

links::
	@echo creating symbolic links...
	@find ../linux/drivers/media -name '*.[ch]' -type f -exec ln -sf '{}' . \;
	@find ../linux/sound -name '*.[ch]' -type f -exec ln -sf '{}' . \;

config-compat.h:: .myconfig
	@perl \
	-e 'print "#ifndef __CONFIG_COMPAT_H__\n";' \
	-e 'print "#define __CONFIG_COMPAT_H__\n\n";' \
	-e 'print "#include <linux/autoconf.h>\n\n";' \
	-e 'while(<>) {' \
	-e '    next unless /^(\S+)\s*:= (\S+)$$/;' \
	-e '    print "#undef $$1\n";' \
	-e '       if($$2 eq "n") { next; }' \
	-e '    elsif($$2 eq "m") { print "#define $$1_MODULE 1\n"; }' \
	-e '    elsif($$2 eq "y") { print "#define $$1 1\n"; }' \
	-e '    else              { print "#define $$1 $$2\n"; }' \
	-e '} print "\n#endif\n";' \
		< .myconfig > config-compat.h

kernel-links makelinks::
	cd ..;	v4l/scripts/makelinks.sh $(KDIR)

#################################################
# Cardlist updating rule

card cardlist cardlists::
	scripts/cardlist

#################################################
# Cleaning rules

clean::
	@find . -name '*.c' -type l -exec rm '{}' \;
	@find . -name '*.h' -type l -exec rm '{}' \;
	-rm -f *~ *.o *.ko .*.o.cmd .*.ko.cmd *.mod.c av7110_firm.h fdump \
		ivtv-svnversion.h config-compat.h

distclean:: clean
	-rm -f .version .*.o.flags .*.o.d Makefile.media \
		Kconfig Kconfig.kern .config .config.cmd .myconfig
	-rm -rf .tmp_versions
	-rm -f scripts/lxdialog scripts/kconfig
	@find .. -name '*.orig' -exec rm '{}' \;
	@find .. -name '*.rej' -exec rm '{}' \;

#################################################
# Kernel module insert/removal rules

start insmod load::
	scripts/rmmod.pl load

stop rmmod unload::
	scripts/rmmod.pl unload

reload::
	scripts/rmmod.pl reload

debug::
	scripts/rmmod.pl debug

#################################################
# Configuration rules

# Kernel config programs
QCONF := $(KDIR)/scripts/kconfig/qconf
GCONF := $(KDIR)/scripts/kconfig/gconf
MCONF := $(KDIR)/scripts/kconfig/mconf
CONF := $(KDIR)/scripts/kconfig/conf

# lxdialog can be in either scripts/lxdialog or scripts/kconfig/lxdialog
LXDIALOG_DIR := $(shell if [ -d $(KDIR)/scripts/kconfig/lxdialog ]; then echo kconfig/ ; fi)
LXDIALOG_LNK := $(if $(LXDIALOG_DIR),scripts/kconfig,scripts/lxdialog)
LXDIALOG := $(KDIR)/scripts/$(LXDIALOG_DIR)lxdialog/lxdialog

.myconfig : .config
	./scripts/make_myconfig.pl

xconfig:: links .version $(QCONF)
	./scripts/make_kconfig.pl $(KDIR)
	$(QCONF) Kconfig

gconfig:: links .version $(GCONF)
	./scripts/make_kconfig.pl $(KDIR)
	$(QCONF) Kconfig

config:: links .version $(CONF)
	./scripts/make_kconfig.pl $(KDIR)
	$(CONF) Kconfig

menuconfig:: links .version $(MCONF) lxdialog
	./scripts/make_kconfig.pl $(KDIR)
	$(MCONF) Kconfig

allyesconfig allmodconfig:: links .version
	./scripts/make_kconfig.pl $(KDIR) 1

# rule to build kernel conf programs
KMAKEVARS := config-targets=1 mixed-targets=0 dot-config=0
$(QCONF) $(GCONF) $(MCONF) $(CONF):
	$(MAKE) -C $(KDIR) -f $(PWD)/Makefile.kernel $(KMAKEVARS) v4l-$(notdir $@)

# lxdialog has two parts, a symlink and the actual binary
.PHONY: lxdialog
lxdialog: $(LXDIALOG) $(LXDIALOG_LNK)

$(LXDIALOG_LNK):
	ln -snf $(KDIR)/$(LXDIALOG_LNK) $(LXDIALOG_LNK)

$(LXDIALOG):
	$(MAKE) -C $(KDIR) -f $(PWD)/Makefile.kernel $(KMAKEVARS) v4l-$(LXDIALOG)

#################################################
# IVTV link rules

ivtv-checkout::
	@if [ ! -d ivtv ]; then \
		echo retrieving the latest ivtv sources from ivtvdriver.org; \
		echo this step requires subversion to be installed; \
		svn co http://ivtvdriver.org/svn/ivtv/trunk ivtv; \
	fi

ivtv-update ivtv-up:: ivtv
	@svn up ivtv

ivtv-links:: ivtv-checkout
	@echo creating ivtv symbolic links...
	@find ivtv/driver -name '*.[ch]' -type f -exec ln -sf '{}' . \;
	@find ivtv/i2c-drivers -name '*.[ch]' -type f -exec ln -sf '{}' . \;

ivtv:: ivtv-links
	@echo '#define IVTV_DRIVER_VERSION_COMMENT ' \
		'"(v4l-dvb + ivtv virtual merge)' \
		"`svn info ivtv | grep Revision`"'"' > ivtv-svnversion.h
	@echo ivtv trunk merged. Run make to build the entire tree.

cx88-ivtv::
	@echo creating cx88-ivtv symbolic links...
	@ln -sf ../v4l_experimental/cx88-ivtv.c .

#################################################
# Old instalation rule

old-install:: rminstall
	@echo -e "\nInstalling new V4L modules at $(DEST)..."

	@strip --strip-debug $(inst-m)

	-install -d $(DEST)
	-install -m 644 -c $(inst-m) $(DEST)

	/sbin/depmod -a ${KERNELRELEASE}

#################################################
# Tree management rules

update::
	make -C .. update

commit cvscommit hgcommit::
	make -C .. commit

push::
	make -C .. push

#################################################
# Help
help::
	@cat ../INSTALL

#################################################
# build tarballs rules

RPWD	:= $(shell cd .. && pwd)
thisdir	:= $(notdir $(RPWD))
name    := v4l-dvb
date    := $(shell date +%Y%m%d)
tardest	?= .

snapdir := $(HOME)/snapshot
snap	:= $(name)

snapshot snap tarball:: update distclean
	echo $(thisdir)
	echo $(name)
	echo $(date) > .snapshot
	(cd ..; tar czf $(snapdir)/$(snap)-$(date).tar.gz .)
	hg history --style scripts/map-changelog > /$(snapdir)/$(snap)-ChangeLog-$(date)
	$(MAKE) -C $(snapdir)
