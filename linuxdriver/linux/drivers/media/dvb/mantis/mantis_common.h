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

#ifndef __MANTIS_COMMON_H
#define __MANTIS_COMMON_H

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/pci.h>
//#include <asm/semaphore.h>

#include "dvbdev.h"
#include "dvb_demux.h"
#include "dmxdev.h"
#include "dvb_frontend.h"
#include "dvb_net.h"
#include <linux/i2c.h>
#include "mantis_reg.h"

#ifdef HAVE_MANTIS_CI
#include "mantis_ca.h"
#endif

#define MANTIS_ERROR		0
#define MANTIS_NOTICE		1
#define MANTIS_INFO		2
#define MANTIS_DEBUG		3

#define dprintk(x, y, z, format, arg...) do {						\
	if (z) {									\
		if	((x > MANTIS_ERROR) && (x > y))					\
			printk(KERN_ERR "%s: " format "\n" , __func__ , ##arg);		\
		else if	((x > MANTIS_NOTICE) && (x > y))				\
			printk(KERN_NOTICE "%s: " format "\n" , __func__ , ##arg);	\
		else if ((x > MANTIS_INFO) && (x > y))					\
			printk(KERN_INFO "%s: " format "\n" , __func__ , ##arg);	\
		else if ((x > MANTIS_DEBUG) && (x > y))					\
			printk(KERN_DEBUG "%s: " format "\n" , __func__ , ##arg);	\
	} else {									\
		if (x > y)								\
			printk(format , ##arg);						\
	}										\
} while(0)

#define mwrite(dat, addr)	writel((dat), addr)
#define mread(addr)		readl(addr)

#define mmwrite(dat, addr)	mwrite((dat), (mantis->mantis_mmio + (addr)))
#define mmread(addr)		mread(mantis->mantis_mmio + (addr))
	
struct mantis_pci {
	// PCI stuff
	u8  latency;

	struct pci_dev *pdev;

	unsigned long mantis_addr;
	volatile void __iomem *mantis_mmio;

	u8  irq;
	u8  revision;

	u16 ts_size;

	// RISC Core
	volatile u32 finished_block;
	volatile u32 last_block;

	u32 block_count;
	u32 block_bytes;
	u32 line_bytes;
	u32 line_count;

	u32 risc_pos;

	u32 buf_size;
	u8  *buf_cpu;
	dma_addr_t buf_dma;

	u32 risc_size;
	u32 *risc_cpu;
	dma_addr_t risc_dma;

	struct	tasklet_struct tasklet;

	struct	i2c_adapter adapter;
	int	i2c_rc;
	wait_queue_head_t	i2c_wq;
	//struct semaphore sem;
	// DVB stuff
	struct  dvb_adapter dvb_adapter;
	struct  dvb_frontend *fe;
	struct  dvb_demux demux;
	struct  dmxdev dmxdev;
	struct  dmx_frontend fe_hw;
	struct  dmx_frontend fe_mem;
	struct  dvb_net dvbnet;

	u8	feeds;

	u32 mantis_int_stat;
	u32 mantis_int_mask;

	struct mantis_ci_state *matis_ca_state;
	volatile int A121314Status;
	// board specific
	u8  mac_address[8];
	u32 sub_vendor_id;
	u32 sub_device_id;

};

extern unsigned int verbose;
extern unsigned int i2c;
extern int mantis_dvb_init(struct mantis_pci *mantis);
extern int mantis_frontend_init(struct mantis_pci *mantis);
extern int mantis_dvb_exit(struct mantis_pci *mantis);
extern void mantis_dma_xfer(unsigned long data);
extern void gpio_set_bits(struct mantis_pci *mantis, u32 bitpos, u8 value);
extern int mantis_fe_reset(struct dvb_frontend *fe);
#endif //__MANTIS_COMMON_H
