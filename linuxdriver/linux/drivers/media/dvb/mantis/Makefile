mantis-objs = mantis_core.o mantis_dma.o mantis_pci.o mantis_i2c.o	\
	mantis_dvb.o mantis_vp1033.o mantis_vp1034.o mantis_vp2033.o	\
	mantis_vp3030.o mantis_vp1041.o


obj-$(CONFIG_DVB_MANTIS) += mantis.o

extra-cflags-$(CONFIG_DVB_MANTIS_CI)+= -DHAVE_MANTIS_CI=1

EXTRA_CFLAGS += $(extra-cflags-y) $(extra-cflags-m)

EXTRA_CFLAGS = -Idrivers/media/dvb/dvb-core/ -Idrivers/media/dvb/frontends/
