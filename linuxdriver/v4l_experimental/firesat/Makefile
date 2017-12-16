obj-$(CONFIG_DVB_FIRESAT)	:= dvb-firesat.o

dvb-firesat-objs := firesat.o avc_api.o cmp.o firesat-rc.o

EXTRA_CFLAGS = -Idrivers/media/dvb/dvb-core/

firesat.c: firesat.h avc_api.h cmp.h firesat-rc.h firesat-ci.h
avc_api.c: avc_api.h firesat.h
cmp.c: cmp.h avc_api.h
firesat-rc.c: firesat-rc.h
firesat-ci.c: firesat-ci.h
