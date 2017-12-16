/*
	CU-1216 driver for the Mantis bridge based cards

	Copyright (C) 2005 Twinhan Technology Co. Ltd
		based on the TDA 10021 driver

	Copyright (C) 1999 Convergence Integrated Media GmbH <ralph@convergence.de>
	Copyright (C) 2004 Markus Schulz <msc@antzsystem.de>

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

#ifndef __CU1216_H
#define __CU1216_H

#include <linux/dvb/frontend.h>
struct cu1216_state {
	struct i2c_adapter *i2c;
	struct dvb_frontend_ops ops;

	//configuration settings 
	const struct cu1216_config *config;
	struct dvb_frontend frontend;

	u8 pwm;
	u8 reg0;

	struct dvb_frontend_parameters params;
};

struct cu1216_config {
	/* the demodulator's i2c address */
	u8 demod_address;

	/* PLL maintenance */
	int (*pll_init)(struct dvb_frontend *fe);
	int (*pll_set)(struct dvb_frontend *fe, struct dvb_frontend_parameters *params);
	int (*fe_reset)(struct dvb_frontend *fe);
};

extern struct dvb_frontend *cu1216_attach(const struct cu1216_config *config, struct i2c_adapter *i2c);


#endif //__MANTIS_CU1216_H
