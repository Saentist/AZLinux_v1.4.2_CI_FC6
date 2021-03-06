/*
	Mantis VP-1034 driver

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
#include "mantis_vp1034.h"

struct mb86a16_config vp1034_config = {
	.demod_address	= 0x08,
	.set_voltage	= vp1034_set_voltage,
};

int vp1034_set_voltage(struct dvb_frontend *fe, fe_sec_voltage_t voltage)
{
	struct mantis_pci *mantis = fe->dvb->priv;

	switch (voltage) {
	case SEC_VOLTAGE_13:
		gpio_set_bits(mantis, 13, 1);
		gpio_set_bits(mantis, 14, 0);
		dprintk(verbose, MANTIS_DEBUG, 1, "SEC_VOLTAGE_13");
		break;
	case SEC_VOLTAGE_18:
		gpio_set_bits(mantis, 13, 1);
		gpio_set_bits(mantis, 14, 1);
		dprintk(verbose, MANTIS_DEBUG, 1, "SEC_VOLTAGE_18");
		break;
	case SEC_VOLTAGE_OFF:
		gpio_set_bits(mantis, 13, 0);
		dprintk(verbose, MANTIS_DEBUG, 1, "Frontend (dummy) POWERDOWN");
		break;
	default:
		dprintk(verbose, MANTIS_DEBUG, 1, "Invalid = (%d)", (u32 ) voltage);
		return -EINVAL;
	}

	return 0;
}
