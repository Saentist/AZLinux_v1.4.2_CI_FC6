/*
 * stb0899.c
 *
 * ST DVB-S2 Frontend Driver (stb0899)
 *
 * Copyright (C) 2001 fnbrd <fnbrd@gmx.de>
 *             & 2002-2004 Andreas Oberritter <obi@linuxtv.org>
 *             & 2003 Wolfram Joost <dbox2@frokaschwei.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef STB0899_H
#define STB0899_H

#include <linux/dvb/frontend.h>
#include "dvb_frontend.h"


struct stb0899_config
{
	u8 demod_address;
	int (*fe_reset)(struct dvb_frontend *fe);
};



//#if defined(CONFIG_DVB_STB0899) || defined(CONFIG_DVB_STB0899_MODULE)
extern struct dvb_frontend* stb0899_attach(const struct stb0899_config* config,struct i2c_adapter* i2c);
/*#else
static inline struct dvb_frontend* stb0899_attach(const struct stb0899_config* config,
						   struct i2c_adapter* i2c)
{
	printk(KERN_WARNING "%s: driver disabled by Kconfig\n", __FUNCTION__);
	return NULL;
}
#endif // CONFIG_DVB_STB0899_H*/

#endif // STB0899_H

