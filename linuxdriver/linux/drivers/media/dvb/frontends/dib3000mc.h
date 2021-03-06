/*
 * Driver for DiBcom DiB3000MC/P-demodulator.
 *
 * Copyright (C) 2004-6 DiBcom (http://www.dibcom.fr/)
 * Copyright (C) 2004-5 Patrick Boettcher (patrick.boettcher\@desy.de)
 *
 * This code is partially based on the previous dib3000mc.c .
 *
 * This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 */
#ifndef DIB3000MC_H
#define DIB3000MC_H

#include "dibx000_common.h"

struct dib3000mc_config {
	struct dibx000_agc_config *agc;

	u8 phase_noise_mode;
	u8 impulse_noise_mode;

	u8  pwm3_inversion;
	u8  use_pwm3;
	u16 pwm3_value;

	u16 max_time;
	u16 ln_adc_level;

	u8 mobile_mode;

	u8 output_mpeg2_in_188_bytes;
};

#define DEFAULT_DIB3000MC_I2C_ADDRESS 16
#define DEFAULT_DIB3000P_I2C_ADDRESS  24

#if defined(CONFIG_DVB_DIB3000MC) || defined(CONFIG_DVB_DIB3000MC_MODULE)
extern int dib3000mc_attach(struct i2c_adapter *i2c_adap, int no_of_demods, u8 default_addr,
    u8 do_i2c_enum, struct dib3000mc_config cfg[], struct dvb_frontend *demod[]);
#else
static inline struct dvb_frontend* dib3000mc_attach(const struct dib3000_config* config,
					     struct i2c_adapter* i2c, struct dib_fe_xfer_ops *xfer_ops)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __FUNCTION__);
	return NULL;
}
#endif // CONFIG_DVB_DIB3000MC

extern struct i2c_adapter * dib3000mc_get_tuner_i2c_master(struct dvb_frontend *demod, int gating);

extern int dib3000mc_pid_control(struct dvb_frontend *fe, int index, int pid,int onoff);
extern int dib3000mc_pid_parse(struct dvb_frontend *fe, int onoff);

extern void dib3000mc_set_config(struct dvb_frontend *, struct dib3000mc_config *);

#endif
