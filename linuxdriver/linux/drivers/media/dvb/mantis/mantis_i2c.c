/*
	Mantis PCI bridge driver

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

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include "mantis_common.h"
//#include <asm/semaphore.h>

#define I2C_HW_B_MANTIS		0x1c

static int mantis_ack_wait(struct mantis_pci *mantis)
{
	int rc = 0;

	if (wait_event_interruptible_timeout(mantis->i2c_wq,
					     mantis->mantis_int_stat & MANTIS_INT_I2CRACK,
					     msecs_to_jiffies(50)) == -ERESTARTSYS)

		rc = -EREMOTEIO;

	// Huge hack for a broken I2CDONE
	if (mantis->mantis_int_stat & MANTIS_INT_I2CRACK)
		msleep_interruptible(2);

	return rc;
}

static int mantis_i2c_pagewrite(struct mantis_pci *mantis, struct i2c_msg *msg)
{
	u8  i;
	u32 txd;
//down(&mantis->sem);
	dprintk(verbose, MANTIS_DEBUG, 1, "Writing to [0x%02x]", msg->addr);
	{
		u32 regInit;
		regInit = mmread(MANTIS_INT_STAT);
		regInit &= (~MANTIS_INT_I2CDONE | ~MANTIS_INT_I2CRACK);
		mmwrite(regInit, MANTIS_INT_STAT);
	}
	// Rate settings
	for (i = 0; i < msg->len; i++) {
		dprintk(verbose, MANTIS_DEBUG, 1, "Data<W[%d]>=[0x%02x]", i, msg->buf[i]);
		txd = (msg->addr << 25) | (msg->buf[i] << 8)
					| MANTIS_I2C_RATE_2
					| MANTIS_I2C_STOP
					| MANTIS_I2C_PGMODE;

		if (i == (msg->len - 1))
			txd &= ~MANTIS_I2C_STOP;

		mmwrite(txd, MANTIS_I2CDATA_CTL);
		if (mantis_ack_wait(mantis) < 0) {
			dprintk(verbose, MANTIS_DEBUG, 1, "ACK failed<W>");
			return -1;
		}
		udelay(500);
	}
//up(&mantis->sem);
	return 0;
}

static int mantis_i2c_pageread(struct mantis_pci *mantis, struct i2c_msg *msg)
{
	u8  i;
	u32 txd, rxd;
//down(&mantis->sem);
	// Rate settings
	for (i = 0; i < msg->len; i++) {
		txd = ((msg->addr << 25) | (1 << 24))
					 | MANTIS_I2C_RATE_2
					 | MANTIS_I2C_STOP
					 | MANTIS_I2C_PGMODE;

		if (i == (msg->len - 1))
			txd &= ~MANTIS_I2C_STOP;
		mmwrite(txd, MANTIS_I2CDATA_CTL);
		if (mantis_ack_wait(mantis) < 0) {
			dprintk(verbose, MANTIS_DEBUG, 1, "ACK failed<R>");
			return -EIO;
		}
		rxd = mmread(MANTIS_I2CDATA_CTL);
		msg->buf[i] = (u8)((rxd >> 8) & 0xFF);
		dprintk(verbose, MANTIS_DEBUG, 1,
			"Data<R[%d]>=[0x%02x]", i, msg->buf[i]);

		udelay(500);
	}
//up(&mantis->sem);
	return 0;
}

static int mantis_i2c_writebyte(struct mantis_pci *mantis,
				u16 addr,
				u8 subaddr,
				u8 data)
{
	u32 txd = 0;
	
	txd = (addr << 25) | (subaddr << 16)
			   | (data << 8)
			   | MANTIS_I2C_RATE_2;

	dprintk(verbose, MANTIS_DEBUG, 1,
		"Writing to [0x%02x],  Data<W[%02x]>=[%02x]",
		addr, subaddr, data);

	mmwrite(txd, MANTIS_I2CDATA_CTL);
	if (mantis_ack_wait(mantis) < 0) {
		dprintk(verbose, MANTIS_DEBUG, 1, "Slave did not ACK !");
		return -EIO;
	}

	return 0;
}

static int mantis_i2c_readbyte(struct mantis_pci *mantis,
			       u16 addr,
			       u8 subaddr,
			       u8 *data)
{
	u32 rxd;
	
	// Rate settings
	rxd = ((addr << 25) | (1 << 24))
			    | (subaddr << 16)
			    | MANTIS_I2C_RATE_2;

	mmwrite(rxd, MANTIS_I2CDATA_CTL);

	// Slave RACK
	if (mantis_ack_wait(mantis) < 0) {
		dprintk(verbose, MANTIS_ERROR, 1, "Slave ACK failed");
		return -EIO;
	}

	rxd = mmread(MANTIS_I2CDATA_CTL);
	udelay(500);
	*data = (rxd >> 8) & 0xff;
	dprintk(verbose, MANTIS_DEBUG, 1, "Reading from [0x%02x], Data<R[0x%02x]>=[0x%02x]",
		addr, subaddr, *data);

	return 0;
}


static int mantis_i2c_xfer(struct i2c_adapter *adapter,
			   struct i2c_msg *msg,
			   int num)
{
	int retval = 0;
	int i;
	struct mantis_pci *mantis;

	mantis = i2c_get_adapdata(adapter);
	if (num == 2) {
		// I2C operation type
		dprintk(verbose, MANTIS_DEBUG, 1, "Num = 2");
		if ((msg[0].flags == 0) && (msg[1].flags == I2C_M_RD)) {
			if ((msg[0].len == 1) && (msg[1].len == 1)) {
				if ((retval = mantis_i2c_readbyte(mantis, msg[0].addr, msg[0].buf[0], &msg[1].buf[0])) < 0)
					return retval;
				return num;
			}
		}
	}

	for (i = 0; i < num; i++) {
		if (msg[i].flags == 0) {
			if (msg[i].len == 2) {
				if ((retval = mantis_i2c_writebyte(mantis, msg[i].addr, msg[i].buf[0], msg[i].buf[1])) < 0)
					return retval;
			} else {
				if ((retval = mantis_i2c_pagewrite(mantis, &msg[i])) < 0)
					return retval;
			}
		} else {
			if ((retval = mantis_i2c_pageread(mantis, &msg[i])) < 0)
				return retval;
		}
	}

	return num;
}

static u32 mantis_i2c_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm mantis_algo = {
	.master_xfer		= mantis_i2c_xfer,
	.functionality		= mantis_i2c_func,
};

static struct i2c_adapter mantis_i2c_adapter = {
	.owner			= THIS_MODULE,
	.name			= "Mantis I2C",
	.id			= I2C_HW_B_MANTIS,
	.class			= I2C_CLASS_TV_DIGITAL,
	.algo			= &mantis_algo,
};

int __devinit mantis_i2c_init(struct mantis_pci *mantis)
{
	u32 intstat;

	memcpy(&mantis->adapter, &mantis_i2c_adapter, sizeof (mantis_i2c_adapter));
	i2c_set_adapdata(&mantis->adapter, mantis);
	mantis->i2c_rc = i2c_add_adapter(&mantis->adapter);
	if (mantis->i2c_rc < 0)
		return mantis->i2c_rc;

	dprintk(verbose, MANTIS_DEBUG, 1, "Initializing I2C ..");

	// Clear all interrupts
	intstat = mmread(MANTIS_INT_STAT);
	mmwrite(intstat, MANTIS_INT_STAT);

	mmwrite(mmread(MANTIS_INT_MASK) | MANTIS_INT_I2CDONE,
					  MANTIS_INT_MASK);

	dprintk(verbose, MANTIS_DEBUG, 1, "INT_STAT=[0x%08x], INT_MASK=[0x%08x]",
		mmread(MANTIS_INT_STAT), mmread(MANTIS_INT_MASK));

	return 0;
}

int __devexit mantis_i2c_exit(struct mantis_pci *mantis)
{
	dprintk(verbose, MANTIS_DEBUG, 1, "Removing I2C adapter");
	return i2c_del_adapter(&mantis->adapter);
}
