deps_config := \
	Kconfig.kern \
	Kconfig

.config include/linux/autoconf.h: $(deps_config)

$(deps_config):
