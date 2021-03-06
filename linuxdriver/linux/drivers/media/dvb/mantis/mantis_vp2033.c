/*
	Mantis VP-2033 driver

	Copyright (C) 2005, 2006 Manu Abraham (abraham.manu@gmail.com)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "mantis_common.h"
#include "mantis_vp2033.h"

struct tda10021_state {
	struct i2c_adapter *i2c;
	struct dvb_frontend_ops ops;
	/* configuration settings */
	const struct tda10021_config *config;
	struct dvb_frontend frontend;

	u8 pwm;
	u8 reg0;
};

struct cu1216_config philips_cu1216_config = {
	.demod_address = 0x18 >> 1,
	.pll_set = philips_cu1216_tuner_set,
	.fe_reset = mantis_fe_reset,
};

int philips_cu1216_tuner_set(struct dvb_frontend *fe,
			     struct dvb_frontend_parameters *params)
{
	struct tda10021_state *state = fe->demodulator_priv;
	u8 buf[4];

	struct i2c_msg msg = {
		.addr = 0xc0 >> 1,
		.flags = 0,
		.buf = buf,
		.len = sizeof (buf)
	};

#define TUNER_MUL 62500

	u32 div = (params->frequency + 36125000 + TUNER_MUL / 2) / TUNER_MUL;

	buf[0] = (div >> 8) & 0x7f;
	buf[1] = div & 0xff;
	buf[2] = 0x86;

        div = div * TUNER_MUL;
        buf[3] = 0x34;

	if (div < (454000000L+36125000L))
		buf[3] = 0x92;

	if (div < (164000000L+36125000L))
		buf[3] = 0xa1;
	
	if (i2c_transfer(state->i2c, &msg, 1) < 0) {
		printk("%s tuner not ack!\n", __FUNCTION__);
		return -EIO;
	}
        
	msleep(100);

	return 0;
}

u8 mantis_fe_type_check(struct dvb_frontend *fe, u8 reg)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8 b0 [] = { reg };
	u8 b1 [] = { 0 };
	struct i2c_msg msg [] = {
		{
			.addr = 0x18 >> 1,
			.flags = 0,
			.buf = b0,
			.len = 1
		},
		{
			.addr = 0x18 >> 1,
			.flags = I2C_M_RD,
			.buf = b1,
			.len = 1
		}
	};
	int ret;

	ret = i2c_transfer (state->i2c, msg, 2);
	if (ret != 2)
		printk("DVB: TDA10021: %s: readreg error (ret == %i)\n",__FUNCTION__, ret);
	return b1[0];

}


