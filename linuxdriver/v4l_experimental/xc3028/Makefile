ifeq ($(obj),)
obj = .
endif

#################################################
# configuration

include $(obj)/Make.config

# symbol exports
export-objs	:= video-buf.o v4l1-compat.o v4l2-common.o
export-objs	+= ir-common.o
ifeq ($(CONFIG_VIDEO_DEV),m)
export-objs     += videodev.o
endif
export-objs	+= bttv-if.o btcx-risc.o
export-objs	+= cx88-cards.o cx88-core.o
export-objs	+= saa7134-core.o saa7134-tvaudio.o

# drivers objects
ir-common-objs  := ir-keymaps.o ir-functions.o
bttv-objs	:= bttv-driver.o bttv-cards.o bttv-risc.o bttv-if.o \
		   bttv-vbi.o bttv-i2c.o bttv-input.o
saa7134-objs	:= saa7134-core.o saa7134-i2c.o saa7134-video.o \
		   saa7134-vbi.o saa7134-tvaudio.o \
		   saa7134-cards.o saa7134-ts.o saa7134-input.o


cx88xx-objs	:= cx88-cards.o cx88-core.o cx88-i2c.o cx88-tvaudio.o \
		   cx88-input.o
cx8800-objs	:= cx88-video.o cx88-vbi.o
cx8802-objs	:= cx88-mpeg.o
tuner-objs	:= tuner-core.o tuner-types.o tuner-simple.o mt20xx.o tda8290.o tea5767.o xc3028.o
msp3400-objs	:= msp3400-driver.o msp3400-kthreads.o
list-multi	:= bttv.o saa7134.o cx88xx.o cx8800.o cx88-alsa.o cx8802.o
em28xx-objs	:= em28xx-video.o em28xx-i2c.o em28xx-cards.o em28xx-core.o \
		   em28xx-input.o
cx25840-objs    := cx25840-core.o cx25840-audio.o cx25840-firmware.o \
		   cx25840-vbi.o
dvb-core-objs	:= dvbdev.o dmxdev.o dvb_demux.o dvb_filter.o \
		   dvb_ca_en50221.o dvb_frontend.o \
		   dvb_net.o dvb_ringbuffer.o
b2c2-flexcop-objs	:= flexcop.o flexcop-fe-tuner.o flexcop-i2c.o \
		   flexcop-sram.o flexcop-eeprom.o flexcop-misc.o \
		   flexcop-hw-filter.o flexcop-dma.o
b2c2-flexcop-pci-objs	:= flexcop-pci.o
b2c2-flexcop-usb-objs	:= flexcop-usb.o
dvb-usb-objs	:= dvb-usb-firmware.o dvb-usb-init.o dvb-usb-urb.o \
		   dvb-usb-i2c.o dvb-usb-dvb.o dvb-usb-remote.o
dvb-usb-vp7045-objs	:= vp7045.o vp7045-fe.o
dvb-usb-vp702x-objs	:= vp702x.o vp702x-fe.o
dvb-usb-dtt200u-objs	:= dtt200u.o dtt200u-fe.o
dvb-usb-dibusb-common-objs	:= dibusb-common.o
dvb-usb-a800-objs	:= a800.o
dvb-usb-dibusb-mb-objs	:= dibusb-mb.o
dvb-usb-dibusb-mc-objs	:= dibusb-mc.o
dvb-usb-nova-t-usb2-objs	:= nova-t-usb2.o
dvb-usb-umt-010-objs	:= umt-010.o
dvb-usb-digitv-objs	:= digitv.o
dvb-usb-cxusb-objs	:= cxusb.o
dvb-ttpci-objs		:= av7110_hw.o av7110_v4l.o av7110_av.o \
			   av7110_ca.o av7110.o av7110_ipack.o av7110_ir.o
saa7146-objs		:= saa7146_i2c.o saa7146_core.o
saa7146_vv-objs		:= saa7146_vv_ksyms.o saa7146_fops.o \
			   saa7146_video.o saa7146_hlp.o saa7146_vbi.o
pvrusb2-objs		:= pvrusb2-i2c-core.o pvrusb2-i2c-cmd-v4l2.o \
			   pvrusb2-audio.o pvrusb2-i2c-chips-v4l2.o \
			   pvrusb2-encoder.o pvrusb2-video-v4l.o \
			   pvrusb2-eeprom.o pvrusb2-tuner.o pvrusb2-demod.o \
			   pvrusb2-main.o pvrusb2-hdw.o pvrusb2-v4l2.o \
			   pvrusb2-sysfs.o pvrusb2-context.o pvrusb2-io.o \
			   pvrusb2-ioread.o pvrusb2-debugifc.o

obj-m				:= video-buf.o v4l1-compat.o v4l2-common.o
obj-m				+= compat_ioctl32.o 
ifeq ($(CONFIG_VIDEO_DEV),m)
obj-m				+= videodev.o
endif

obj-$(CONFIG_VIDEO_BTTV)	+= btcx-risc.o ir-common.o bttv.o tveeprom.o
obj-$(CONFIG_VIDEO_CX88)	+= btcx-risc.o cx88xx.o cx8800.o cx8802.o \
				   cx88-blackbird.o tveeprom.o
obj-$(CONFIG_TVP5150)		+= tvp5150.o
obj-$(CONFIG_SAA711X)		+= saa711x.o
obj-$(CONFIG_EM28XX)		+= em28xx.o tveeprom.o
obj-$(CONFIG_VIDEO_DECODER)	+= saa7115.o cx25840.o saa7127.o bt832.o

ifeq ($(CONFIG_VIDEO_ALSA),y)
  EXTRA_CFLAGS += -DUSING_CX88_ALSA=1
  obj-$(CONFIG_VIDEO_CX88)	+= cx88-alsa.o
  obj-$(CONFIG_VIDEO_SAA7134)	+= saa7134-alsa.o 
## This one is installed by ALSA as snd_bt87x
#  obj-$(CONFIG_VIDEO_BTTV)	+= bt87x.o 
endif

obj-$(CONFIG_VIDEO_SAA7134)	+= saa7134.o saa7134-empress.o saa6752hs.o
obj-$(CONFIG_VIDEO_SAA7134)	+= saa7134-oss.o
obj-$(CONFIG_VIDEO_IR)		+= ir-common.o 
obj-$(CONFIG_VIDEO_TUNER)	+= tuner.o tda9887.o saa6588.o
obj-$(CONFIG_VIDEO_TVAUDIO)	+= msp3400.o tvaudio.o tvmixer.o wm8775.o \
				   cs53l32a.o tda7432.o tda9875.o

obj-$(CONFIG_VIDEO_CX88_DVB)	+= video-buf-dvb.o cx88-dvb.o cx88-vp3054-i2c.o
obj-$(CONFIG_VIDEO_SAA7134_DVB) += video-buf-dvb.o saa7134-dvb.o
obj-$(CONFIG_DVB_BT8XX)		+= bt878.o dvb-bt8xx.o dst.o dst_ca.o
obj-$(CONFIG_DVB_FRONTENDS)	+= dvb-pll.o cx22702.o or51132.o lgdt330x.o \
				   mt352.o tda1004x.o sp887x.o nxt6000.o \
				   cx24110.o or51211.o nxt200x.o cx24123.o \
				   mt312.o stv0299.o bcm3510.o \
				   dib3000-common.o dib3000mb.o dib3000mc.o \
				   ves1820.o cx22700.o tda8083.o ves1x93.o \
				   stv0297.o sp8870.o l64781.o s5h1420.o \
				   tda10021.o at76c651.o tda80xx.o
obj-$(CONFIG_DVB_CORE)		+= dvb-core.o
obj-$(CONFIG_DVB_B2C2_FLEXCOP)	+= b2c2-flexcop.o
obj-$(CONFIG_DVB_B2C2_FLEXCOP)	+= stv0297_cs2.o
obj-$(CONFIG_DVB_B2C2_FLEXCOP_PCI) += b2c2-flexcop-pci.o
obj-$(CONFIG_DVB_B2C2_FLEXCOP_USB) += b2c2-flexcop-usb.o
obj-$(CONFIG_DVB_CINERGYT2)	+= cinergyT2.o
obj-$(CONFIG_DVB_PLUTO2)	+= pluto2.o
obj-$(CONFIG_DVB_TTUSB_BUDGET)	+= dvb-ttusb-budget.o
obj-$(CONFIG_DVB_TTUSB_DEC)	+= ttusb_dec.o ttusbdecfe.o
obj-$(CONFIG_DVB_USB)		+= dvb-usb.o
obj-$(CONFIG_DVB_USB_VP7045)	+= dvb-usb-vp7045.o
obj-$(CONFIG_DVB_USB_VP702X)	+= dvb-usb-vp702x.o
obj-$(CONFIG_DVB_USB_DTT200U)	+= dvb-usb-dtt200u.o
obj-$(CONFIG_DVB_USB_A800)	+= dvb-usb-dibusb-common.o dvb-usb-a800.o
obj-$(CONFIG_DVB_USB_DIBUSB_MB)	+= dvb-usb-dibusb-common.o dvb-usb-dibusb-mb.o
obj-$(CONFIG_DVB_USB_DIBUSB_MC)	+= dvb-usb-dibusb-common.o dvb-usb-dibusb-mc.o
obj-$(CONFIG_DVB_USB_NOVA_T_USB2)	+= dvb-usb-dibusb-common.o \
					   dvb-usb-nova-t-usb2.o
obj-$(CONFIG_DVB_USB_UMT_010)	+= dvb-usb-dibusb-common.o dvb-usb-umt-010.o
obj-$(CONFIG_DVB_USB_DIGITV)	+= dvb-usb-digitv.o
obj-$(CONFIG_DVB_USB_CXUSB)	+= dvb-usb-cxusb.o
obj-$(CONFIG_DVB_BUDGET)	+= budget-core.o budget.o ttpci-eeprom.o
obj-$(CONFIG_DVB_BUDGET_AV)	+= budget-core.o budget-av.o ttpci-eeprom.o
obj-$(CONFIG_DVB_BUDGET_CI)	+= budget-core.o budget-ci.o ttpci-eeprom.o
obj-$(CONFIG_DVB_BUDGET_PATCH)	+= budget-core.o budget-patch.o ttpci-eeprom.o
obj-$(CONFIG_DVB_AV7110)	+= dvb-ttpci.o ttpci-eeprom.o
obj-$(CONFIG_VIDEO_SAA7146)	+= saa7146.o
obj-$(CONFIG_VIDEO_SAA7146_VV)	+= saa7146_vv.o
obj-$(CONFIG_VIDEO_PVRUSB2)	+= pvrusb2.o

# 2.6-only stuff
ifeq ($(VERSION).$(PATCHLEVEL),2.6)
  ifeq ($(CONFIG_VIDEO_BTTV),m)
    bttv-objs			+= bttv-gpio.o
    obj-$(CONFIG_VIDEO_IR)	+= ir-kbd-i2c.o 
  endif
  ifeq ($(CONFIG_VIDEO_SAA7134),m)
    obj-$(CONFIG_VIDEO_IR)	+= ir-kbd-i2c.o 
  endif
endif

ifeq ($(CONFIG_VIDEO_ADV_DEBUG),y)
 EXTRA_CFLAGS += -DCONFIG_VIDEO_ADV_DEBUG=1
endif

# for DVB
EXTRA_CFLAGS += -DDVB_CVS=1
EXTRA_CFLAGS += -I$(srctree)/drivers/media/dvb/dvb-core/
EXTRA_CFLAGS += -I$(srctree)/drivers/media/dvb/frontends/
ifeq ($(CONFIG_VIDEO_CX88_DVB),m)
 EXTRA_CFLAGS += -DCONFIG_VIDEO_CX88_DVB_MODULE=1
 EXTRA_CFLAGS += -DHAVE_CX22702=1
 EXTRA_CFLAGS += -DHAVE_OR51132=1
 EXTRA_CFLAGS += -DHAVE_LGDT330X=1
 EXTRA_CFLAGS += -DHAVE_MT352=1
 EXTRA_CFLAGS += -DHAVE_NXT200X=1
 EXTRA_CFLAGS += -DHAVE_CX24123=1
 EXTRA_CFLAGS += -DHAVE_VP3054_I2C=1
endif
ifeq ($(CONFIG_VIDEO_SAA7134_DVB),m)
 EXTRA_CFLAGS += -DHAVE_MT352=1
 EXTRA_CFLAGS += -DHAVE_TDA1004X=1
 EXTRA_CFLAGS += -DHAVE_NXT200X=1
endif
ifeq ($(CONFIG_VIDEO_BUF_DVB),m)
 EXTRA_CFLAGS += -DHAVE_VIDEO_BUF_DVB=1
endif
ifeq ($(CONFIG_DVB_AV7110_OSD),y)
 EXTRA_CFLAGS += -DCONFIG_DVB_AV7110_OSD=1
endif
ifeq ($(CONFIG_DVB_AV7110_FIRMWARE),y)
src := $(PWD)

$(src)/av7110.c:	$(obj)/av7110_firm.h

$(obj)/fdump:
	$(CC) -o $@ $(src)/fdump.c

$(obj)/av7110_firm.h:	$(obj)/fdump
	$(obj)/fdump $(CONFIG_DVB_AV7110_FIRMWARE_FILE) dvb_ttpci_fw $@
endif


#################################################
# compile modules

ifneq ($(KERNELRELEASE),)

ifeq ($(VERSION).$(PATCHLEVEL),2.6)
 export-objs	:=
 list-multi	:=
else
 multi-m	:= $(filter $(list-multi), $(obj-m))
 int-m		:= $(sort $(foreach m, $(multi-m), $($(basename $(m))-objs)))
 export-objs	:= $(filter $(int-m) $(obj-m),$(export-objs))
endif

CC += -I$(obj)
EXTRA_CFLAGS += -g

EXTRA_CFLAGS += $(if $(wildcard $(srctree)/.mm), -DMM_KERNEL)

snapshot := $(wildcard $(obj)/.snapshot)
ifneq ($(snapshot),)
SNAPSHOT_CFLAGS	:= -DSNAPSHOT='$(shell cat $(snapshot))'
EXTRA_CFLAGS	+= $(SNAPSHOT_CFLAGS)
endif

#
# Need to compile with kernel 2.4
#
bttv.o: $(bttv-objs)
	$(LD) -r -o $@ $(bttv-objs)

cx25840.o: $(cx25840-objs)
	$(LD) -g -r -o $@ $(cx25840-objs)

cx8800.o: $(cx8800-objs)
	$(LD) -g -r -o $@ $(cx8800-objs)

cx8802.o: $(cx8802-objs)
	$(LD) -g -r -o $@ $(cx8802-objs)

cx88xx.o: $(cx88xx-objs)
	$(LD) -g -r -o $@ $(cx88xx-objs)

em28xx.o: $(em28xx-objs)
	$(LD) -g -r -o $@ $(em28xx-objs)

saa7134.o: $(saa7134-objs)
	$(LD) -g -r -o $@ $(saa7134-objs)

tuner.o: $(tuner-objs)
	$(LD) -g -r -o $@ $(tuner-objs)

-include $(TOPDIR)/Rules.make
else
# take version info from last module build if available
-include $(obj)/.version
endif

KDIR		:= /lib/modules/$(KERNELRELEASE)/build
PWD		:= $(shell pwd)
DEST		:= /lib/modules/$(KERNELRELEASE)/$(MDIR)
KDIR26		:= /lib/modules/$(KERNELRELEASE)/kernel/drivers/media
KDIRUSB		:= /lib/modules/$(KERNELRELEASE)/kernel/drivers/usb/media

ifeq ($(VERSION).$(PATCHLEVEL),2.6)
 CPPFLAGS := -I$(SUBDIRS)/../linux/include $(CPPFLAGS) -I$(SUBDIRS)/
 MYCFLAGS :=
else
 MYCFLAGS := CFLAGS="-I../linux/include -D__KERNEL__ -I$(KDIR)/include -DEXPORT_SYMTAB"
endif

# which files to install?
inst-m		:= $(wildcard *.ko)
ifeq ($(inst-m),)
  inst-m	:= $(obj-m)
endif

inst_common := ir-common.ko
inst_video := btcx-risc.ko bttv.ko tda9887.ko tuner.ko tvaudio.ko tveeprom.ko saa6588.ko
#inst_video += bt87x.ko
inst_video += tvmixer.ko v4l1-compat.ko v4l2-common.ko wm8775.ko cs53l32a.ko
inst_video += video-buf.ko video-buf-dvb.ko 
ifeq ($(CONFIG_VIDEO_DEV),m)
inst_video += videodev.ko
endif
inst_video += ir-kbd-i2c.ko msp3400.ko
inst_video += tvp5150.ko saa711x.ko saa7134-alsa.ko saa7134-oss.ko
inst_video += saa7115.ko cx25840.ko saa7127.ko compat_ioctl32.ko
inst_cx88 := cx8800.ko cx8802.ko cx88-alsa.ko 
inst_cx88 += cx88-blackbird.ko cx88xx.ko cx88-dvb.ko cx88-vp3054-i2c.ko
inst_saa7134 := saa6752hs.ko saa7134.ko saa7134-empress.ko saa7134-dvb.ko
inst_em28xx := em28xx.ko
inst_bt8xx := bt878.ko dvb-bt8xx.ko dst.ko dst_ca.ko
inst_frontends := cx22702.ko dvb-pll.ko lgdt330x.ko or51132.ko tda1004x.ko
inst_frontends += mt352.ko sp887x.ko nxt6000.ko cx24110.ko or51211.ko
inst_frontends += nxt200x.ko cx24123.ko mt312.ko stv0299.ko
inst_frontends += bcm3510.ko dib3000mb.ko dib3000mc.ko ves1820.ko 
inst_frontends += cx22700.ko tda8083.ko ves1x93.ko stv0297.ko sp8870.ko 
inst_frontends += l64781.ko s5h1420.ko tda10021.ko at76c651.ko tda80xx.ko
inst_frontends += stv0297_cs2.ko
inst_dvb-core := dvb-core.ko
inst_saa7146 := saa7146.ko saa7146_vv.ko
inst_b2c2 := b2c2-flexcop.ko b2c2-flexcop-pci.ko b2c2-flexcop-usb.ko
inst_cinergyT2 := cinergyT2.ko
inst_dvb-usb := dvb-usb.ko dvb-usb-vp7045.ko dvb-usb-vp702x.ko
inst_dvb-usb += dvb-usb-dtt200u.ko dvb-usb-a800.ko dvb-usb-dibusb-mb.ko
inst_dvb-usb += dvb-usb-dibusb-mc.ko dvb-usb-nova-t-usb2.ko
inst_dvb-usb += dvb-usb-umt-010.ko dvb-usb-digitv.ko dvb-usb-cxusb.ko
inst_pluto2 := pluto2.ko
inst_ttpci := budget-core.ko budget.ko ttpci-eeprom.ko budget-av.ko
inst_ttpci += budget-ci.ko budget-patch.ko dvb-ttpci.ko
inst_ttusb-budget := dvb-ttusb-budget.ko
inst_ttusb-dec := ttusb_dec.ko ttusbdecfe.ko
inst_pvrusb2 := pvrusb2.ko

v4l_modules := $(shell /sbin/lsmod|cut -d' ' -f1 ) $(patsubst %.ko,%,$(inst-m))

# locales seem to cause trouble sometimes.
LC_ALL = POSIX
export LC_ALL

default:: links .version
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) $(MYCFLAGS) modules

pvrusb2::
	@scripts/merge-pvrusb2.sh

pvrusb2-unmerge::
#	patch -R -p1 < scripts/merge-pvrusb2.sh

links::
	@echo creating symbolic links...
	@find ../linux/drivers/media -name '*.[c,h]' -type f -exec ln -sf '{}' . \;
	@find ../linux/sound -name '*.[c,h]' -type f -exec ln -sf '{}' . \;

#	@find ../linux/include -name '*.[h]' -type f -exec ln -sf '{}' . \;

kernel-links makelinks::
	cd ..;	v4l/scripts/makelinks.sh $(KDIR)

install:: v4l-install dvb-install

v4l-install:: v4l-rminstall
	@echo -e "\nInstalling new V4L modules at corresponding Kernel dir..."

	@strip --strip-debug $(inst-m)

	-install -d $(KDIR26)/common
	-install -m 644 -c $(inst_common) $(KDIR26)/common

	-install -d $(KDIR26)/video
	-install -m 644 -c $(inst_video) $(KDIR26)/video

	-install -d $(KDIR26)/video/cx88
	-install -m 644 -c $(inst_cx88) $(KDIR26)/video/cx88

	-install -d $(KDIR26)/video/saa7134
	-install -m 644 -c $(inst_saa7134) $(KDIR26)/video/saa7134

	-install -d $(KDIR26)/video/em28xx
	-install -m 644 -c $(inst_em28xx) $(KDIR26)/video/em28xx

	-install -d $(KDIR26)/video/pvrusb2
	-install -m 644 -c $(inst_pvrusb2) $(KDIR26)/video/pvrusb2

#	-install -d $(KDIRUSB)
#	-install -m 644 -c $(inst_usb) $(KDIRUSB)/

	/sbin/depmod -a ${KERNELRELEASE}

dvb-install:: dvb-rminstall
	@echo -e "\nInstalling new DVB modules at corresponding Kernel dir..."

	@strip --strip-debug $(inst-m)

	-install -d $(KDIR26)/common
	-install -m 644 -c $(inst_saa7146) $(KDIR26)/common

	-install -d $(KDIR26)/dvb/frontends
	-install -m 644 -c $(inst_frontends) $(KDIR26)/dvb/frontends

	-install -d $(KDIR26)/dvb/bt8xx
	-install -m 644 -c $(inst_bt8xx) $(KDIR26)/dvb/bt8xx

	-install -d $(KDIR26)/dvb/dvb-core
	-install -m 644 -c $(inst_dvb-core) $(KDIR26)/dvb/dvb-core

	-install -d $(KDIR26)/dvb/b2c2
	-install -m 644 -c $(inst_b2c2) $(KDIR26)/dvb/b2c2

	-install -d $(KDIR26)/dvb/cinergyT2
	-install -m 644 -c $(inst_cinergyT2) $(KDIR26)/dvb/cinergyT2

	-install -d $(KDIR26)/dvb/dvb-usb
	-install -m 644 -c $(inst_dvb-usb) $(KDIR26)/dvb/dvb-usb

	-install -d $(KDIR26)/dvb/pluto2
	-install -m 644 -c $(inst_pluto2) $(KDIR26)/dvb/pluto2

	-install -d $(KDIR26)/dvb/ttpci
	-install -m 644 -c $(inst_ttpci) $(KDIR26)/dvb/ttpci

	-install -d $(KDIR26)/dvb/ttusb-budget
	-install -m 644 -c $(inst_ttusb-budget) $(KDIR26)/dvb/ttusb-budget

	-install -d $(KDIR26)/dvb/ttusb-dec
	-install -m 644 -c $(inst_ttusb-dec) $(KDIR26)/dvb/ttusb-dec

	/sbin/depmod -a ${KERNELRELEASE}

old-install:: rminstall
	@echo -e "\nInstalling new V4L modules at $(DEST)..."

	@strip --strip-debug $(inst-m)

	-install -d $(DEST)
	-install -m 644 -c $(inst-m) $(DEST)

	/sbin/depmod -a ${KERNELRELEASE}

rminstall:: v4l-rminstall dvb-rminstall

v4l-rminstall::
	@echo -e "\nEliminating old V4L modules (errors on this step is not a problem).."

	-@rm -r $(DEST) \
		$(addprefix $(KDIR26)/common/, $(inst_common)) \
		$(addprefix $(KDIR26)/video/, $(inst_video)) \
		$(addprefix $(KDIR26)/video/cx88/, $(inst_cx88)) \
		$(addprefix $(KDIR26)/video/saa7134/, $(inst_saa7134)) \
		$(addprefix $(KDIR26)/video/em28xx/, $(inst_em28xx)) \
		$(addprefix $(KDIRUSB)/, $(inst_em28xx)) \
		$(addprefix $(KDIR26)/common/, $(addsuffix .gz,$(inst_common))) \
		$(addprefix $(KDIR26)/video/, $(addsuffix .gz,$(inst_video))) \
		$(addprefix $(KDIR26)/video/cx88/, $(addsuffix .gz,$(inst_cx88))) \
		$(addprefix $(KDIR26)/video/saa7134/, $(addsuffix .gz,$(inst_saa7134))) 2>/dev/null \
		$(addprefix $(KDIR26)/video/pvrusb2/, $(addsuffix .gz,$(inst_pvrusb2))) 2>/dev/null \
		$(addprefix $(KDIR26)/video/em28xx/, $(addsuffix .gz,$(inst_em28xx))) 2>/dev/null \
		$(addprefix $(KDIRUSB)/, $(addsuffix .gz,$(inst_em28xx))) 2>/dev/null
	@echo

dvb-rminstall::
	@echo -e "\nEliminating old DVB modules (errors on this step is not a problem).."

	-@rm -r $(DEST) \
		$(addprefix $(KDIR26)/dvb/bt8xx/, $(inst_bt8xx)) \
		$(addprefix $(KDIR26)/dvb/frontends/, $(inst_frontends)) \
		$(addprefix $(KDIR26)/dvb/dvb-core/, $(inst_dvb-core)) \
		$(addprefix $(KDIR26)/dvb/b2c2/, $(inst_b2c2)) \
		$(addprefix $(KDIR26)/dvb/cinergyT2/, $(inst_cinergyT2)) \
		$(addprefix $(KDIR26)/dvb/dvb-usb/, $(inst_dvb-usb)) \
		$(addprefix $(KDIR26)/dvb/pluto2/, $(inst_pluto2)) \
		$(addprefix $(KDIR26)/dvb/ttpci/, $(inst_ttpci)) \
		$(addprefix $(KDIR26)/dvb/ttusb-budget/, $(inst_ttusb-budget)) \
		$(addprefix $(KDIR26)/dvb/ttusb-dec/, $(inst_ttusb-dec)) \
		$(addprefix $(KDIR26)/dvb/bt8xx/, $(addsuffix .gz,$(inst_bt8xx))) \
		$(addprefix $(KDIR26)/dvb/frontends/, $(addsuffix .gz,$(inst_frontends))) \
		$(addprefix $(KDIR26)/dvb/dvb-core/, $(addsuffix .gz,$(inst_dvb-core))) \
		$(addprefix $(KDIR26)/dvb/b2c2/, $(addsuffix .gz,$(inst_b2c2))) \
		$(addprefix $(KDIR26)/dvb/cinergyT2/, $(addsuffix .gz,$(inst_cinergyT2))) \
		$(addprefix $(KDIR26)/dvb/dvb-usb/, $(addsuffix .gz,$(inst_dvb-usb))) \
		$(addprefix $(KDIR26)/dvb/pluto2/, $(addsuffix .gz,$(inst_pluto2))) \
		$(addprefix $(KDIR26)/dvb/ttpci/, $(addsuffix .gz,$(inst_ttpci))) \
		$(addprefix $(KDIR26)/dvb/ttusb-budget/, $(addsuffix .gz,$(inst_ttusb-budget))) \
		$(addprefix $(KDIR26)/dvb/ttusb-dec/, $(addsuffix .gz,$(inst_ttusb-dec))) 2>/dev/null
	@echo

rmmodules::
	rmmod -w $(shell echo "$(v4l_modules)"|sed s,' ','\n',g|sed s,'-','_',g|sort|uniq -d)

clean::
	@find . -name '*.c' -type l -exec rm '{}' \;
	@find . -name '*.h' -type l -exec rm '{}' \;
	-rm -f *~ *.o *.ko .*.o.cmd .*.ko.cmd *.mod.c av7110_firm.h fdump

distclean:: clean
	-rm -f .version .*.o.flags .*.o.d
	-rm -rf .tmp_versions
	@find .. -name '*.orig' -exec rm '{}' \;
	@find .. -name '*.rej' -exec rm '{}' \;

change changes changelog::
	make -C .. changelog

cvscommit cvs commit::
	cd ..; cvs commit

update v4l-update::
	cd ..; cvs update

insmod load::
	scripts/insmod.sh load

rmmod unload::
	scripts/insmod.sh unload

reload::
	scripts/insmod.sh reload

.version:
ifneq ($(KERNELRELEASE),)
	echo -e VERSION=$(VERSION)\\nPATCHLEVEL:=$(PATCHLEVEL)\\nSUBLEVEL:=$(SUBLEVEL)\\nKERNELRELEASE:=$(KERNELRELEASE) > $(obj)/.version
else
	echo "No version yet."
	uname -r|perl -ne 'if (/^([0-9]*)\.([0-9])*\.([0-9]*)(.*)$$/) { printf ("VERSION=%s\nPATCHLEVEL:=%s\nSUBLEVEL:=%s\nKERNELRELEASE:=%s.%s.%s%s\n",$$1,$$2,$$3,$$1,$$2,$$3,$$4); };' > $(obj)/.version

endif

#################################################
# build tarballs

RPWD	:= $(shell cd .. && pwd)
thisdir	:= $(notdir $(RPWD))
name    := $(thisdir)
date    := $(shell date +%Y%m%d)
tardest	?= .

snapdir := $(HOME)/snapshot
snap	:= $(name)

snapshot snap tarball: update distclean
	echo $(thisdir)
	echo $(name)
	echo $(date) > .snapshot
	(cd ..; tar czf $(snapdir)/$(snap)-$(date).tar.gz .)
	cp ChangeLog /$(snapdir)/$(snap)-ChangeLog-$(date)
	$(MAKE) -C $(snapdir)


#################################################
# other stuff

%.asm: %.o
	objdump -S $< > $@

card cardlist cardlists:
	scripts/cardlist

