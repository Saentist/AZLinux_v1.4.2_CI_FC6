/*
 * FireSAT DVB driver
 *
 * Copyright (c) 2004 Andreas Monitzer <andy@monitzer.com>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 */
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/errno.h>
#include <asm/semaphore.h>
#include <ieee1394_hotplug.h>
#include <nodemgr.h>
#include <highlevel.h>
#include <ohci1394.h>
#include <hosts.h>

/*#include "dvb_frontend.h"
#include "dmxdev.h"
#include "dvb_demux.h"
#include "dvb_net.h"

#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>*/
#include "firesat.h"
#include "avc_api.h"
#include "cmp.h"
#include "firesat-rc.h"
#include "firesat-ci.h"

#define FIRESAT_Vendor_ID   0x001287

/*** HOTPLUG STUFF **********************************************************/
/*
 * Export information about protocols/devices supported by this driver.
 */
static struct ieee1394_device_id firesat_id_table[] = {
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
	.model_id       = 0x11,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
	.model_id       = 0x12,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x13,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x14,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x21,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x22,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x23,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x24,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x25,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x26,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x27,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x34,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x35,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{
		.match_flags	= /*IEEE1394_MATCH_VENDOR_ID |*/ IEEE1394_MATCH_MODEL_ID | IEEE1394_MATCH_SPECIFIER_ID | IEEE1394_MATCH_VERSION,
//        .vendor_id      = FIRESAT_Vendor_ID & 0xffffff,
//        .vendor_id      = 0x000000,
	.model_id       = 0x36,
	.specifier_id   = AVC_UNIT_SPEC_ID_ENTRY & 0xffffff,
	.version        = AVC_SW_VERSION_ENTRY & 0xffffff
	},
	{ }
};

MODULE_DEVICE_TABLE(ieee1394, firesat_id_table);

/* list of all firesat devices */
LIST_HEAD(firesat_list);
spinlock_t firesat_list_lock = SPIN_LOCK_UNLOCKED;

static void firesat_add_host (struct hpsb_host *host);
static void firesat_remove_host (struct hpsb_host *host);
static void firesat_host_reset(struct hpsb_host *host);
static void iso_receive(struct hpsb_host *host, int channel, quadlet_t *data,
			size_t length);
static void fcp_request(struct hpsb_host *host, int nodeid, int direction,
						int cts, u8 *data, size_t length);

static struct hpsb_highlevel firesat_highlevel = {
	.name =         "FireSAT",
	.add_host =     firesat_add_host,
	.remove_host =  firesat_remove_host,
	.host_reset =   firesat_host_reset,
	.iso_receive =	iso_receive,
	.fcp_request =	fcp_request,
};


static struct dvb_frontend_info firesat_S_frontend_info = {
	.name			= "FireSAT DVB-S Frontend",
	.type			= FE_QPSK,
	.frequency_min		= 950000,
	.frequency_max		= 2150000,
	.frequency_stepsize	= 125,
	.symbol_rate_min	= 1000000,
	.symbol_rate_max	= 40000000,
	.caps = FE_CAN_INVERSION_AUTO | FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 |
		FE_CAN_FEC_3_4 | FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 |
		FE_CAN_FEC_AUTO | FE_CAN_QPSK,
};

static struct dvb_frontend_info firesat_C_frontend_info = {
	.name			= "FireSAT DVB-C Frontend",
	.type			= FE_QAM,
	.frequency_min		= 47000000,
	.frequency_max		= 866000000,
	.frequency_stepsize	= 62500,
	.symbol_rate_min	= 870000,
	.symbol_rate_max	= 6900000,
	.caps = FE_CAN_INVERSION_AUTO | FE_CAN_QAM_16 | FE_CAN_QAM_32 |
		FE_CAN_QAM_64 | FE_CAN_QAM_128 | FE_CAN_QAM_256 |
		FE_CAN_QAM_AUTO,
};

static struct dvb_frontend_info firesat_T_frontend_info = {
	.name			= "FireSAT DVB-T Frontend",
	.type			= FE_OFDM,
	.frequency_min		= 49000000,
	.frequency_max		= 861000000,
	.frequency_stepsize	= 62500,
	.caps = FE_CAN_INVERSION_AUTO | FE_CAN_FEC_2_3 |
		FE_CAN_TRANSMISSION_MODE_AUTO | FE_CAN_GUARD_INTERVAL_AUTO |
		FE_CAN_HIERARCHY_AUTO,
};

static void firesat_add_host (struct hpsb_host *host) {
	struct ti_ohci *ohci;
//    printk(KERN_INFO "FireSAT add_host (nodeid = 0x%x)\n",host->node_id);

	/* We only work with the OHCI-1394 driver */
	if (strcmp(host->driver->name, OHCI1394_DRIVER_NAME))
		return;

	ohci = (struct ti_ohci *)host->hostdata;

	if (!hpsb_create_hostinfo(&firesat_highlevel, host, 0)) {
		printk(KERN_ERR "Cannot allocate hostinfo\n");
		return;
	}

	hpsb_set_hostinfo(&firesat_highlevel, host, ohci);
	hpsb_set_hostinfo_key(&firesat_highlevel, host, ohci->host->id);
}

static void firesat_remove_host (struct hpsb_host *host) {
//    printk(KERN_INFO "FireSAT remove_host (nodeid = 0x%x)\n",host->node_id);
}

static void firesat_host_reset(struct hpsb_host *host) {
    printk(KERN_INFO "FireSAT host_reset (nodeid = 0x%x, hosts active = %d)\n",host->node_id,host->nodes_active);
}

struct firewireheader {
    union {
	struct {
	    unsigned char tcode:4;
	    unsigned char sy:4;
	    unsigned char tag:2;
	    unsigned char channel:6;

	    unsigned char length_l;
	    unsigned char length_h;
	} hdr;
	unsigned long val;
    };
};

struct CIPHeader {
    union {
	struct {
	    unsigned char syncbits:2;
	    unsigned char sid:6;
	    unsigned char dbs;
	    unsigned char fn:2;
	    unsigned char qpc:3;
	    unsigned char sph:1;
	    unsigned char rsv:2;
	    unsigned char dbc;
	    unsigned char syncbits2:2;
	    unsigned char fmt:6;
	    unsigned long fdf:24;
	} cip;
	unsigned long long val;
    };
};

struct MPEG2Header {
    union {
	struct {
	    unsigned char sync; // must be 0x47
	    unsigned char transport_error_indicator:1;
	    unsigned char payload_unit_start_indicator:1;
	    unsigned char transport_priority:1;
	    unsigned short pid:13;
	    unsigned char transport_scrambling_control:2;
	    unsigned char adaption_field_control:2;
	    unsigned char continuity_counter:4;
	} hdr;
	unsigned long val;
    };
};

static void iso_receive(struct hpsb_host *host, int channel, quadlet_t *data,
			size_t length) {
//    printk(KERN_INFO "FireSAT iso_receive: channel %d, length = %d\n", channel, length);

	if(length <= 12)
		return; // ignore empty packets
	else {
		struct firesat *firesat=NULL;
		struct firesat *firesat_entry;
		unsigned long flags;

		spin_lock_irqsave(&firesat_list_lock, flags);
		list_for_each_entry(firesat_entry,&firesat_list,list) {
			if(firesat_entry->host == host && firesat_entry->isochannel == channel) {
				firesat=firesat_entry;
				break;
			}
		}
		spin_unlock_irqrestore(&firesat_list_lock, flags);

		if(firesat) {
			char *buf=((char*)data) + sizeof(struct firewireheader)+sizeof(struct CIPHeader);
			int count = (length-sizeof(struct CIPHeader)) / 192;

//			printk(KERN_INFO "%s: length = %u\n data[0] = %08x\n data[1] = %08x\n data[2] = %08x\n data[3] = %08x\n data[4] = %08x\n",__FUNCTION__, length, data[0],data[1],data[2],data[3],data[4]);

			while (count--) {

				if(buf[sizeof(quadlet_t) /*timestamp*/] == 0x47)
					dvb_dmx_swfilter_packet(&firesat->demux, &buf[sizeof(quadlet_t)]);
				else
					printk("%s: invalid packet, skipping\n", __FUNCTION__);
				buf += 188 + sizeof(quadlet_t) /* timestamp */;
			}
		}
	}
}

static void fcp_request(struct hpsb_host *host, int nodeid, int direction,
			int cts, u8 *data, size_t length) {
//	printk(KERN_INFO "FireSAT fcp_request length=%d\n",length);
	if(length>0 && ((data[0] & 0xF0) >> 4) == 0) {
		struct firesat *firesat=NULL;
		struct firesat *firesat_entry;
		unsigned long flags;

		spin_lock_irqsave(&firesat_list_lock, flags);
		list_for_each_entry(firesat_entry,&firesat_list,list) {
			if(firesat_entry->host == host && firesat_entry->nodeentry->nodeid == nodeid && (firesat_entry->subunit == (data[1]&0x7) || (firesat_entry->subunit == 0 && (data[1]&0x7) == 0x7))) {
				firesat=firesat_entry;
				break;
			}
		}
		spin_unlock_irqrestore(&firesat_list_lock, flags);

		if(firesat)
			AVCRecv(firesat,data,length);
		else
			printk("%s: received fcp request from unknown source, ignored\n",__FUNCTION__);
	} // else ignore
}

static int firesat_frontend_ioctl(struct dvb_frontend *frontend,
				     unsigned int cmd, void *arg) {
	struct firesat *firesat=frontend->data;

	switch(cmd) {
	case FE_INIT:
		firesat->isochannel = firesat->adapter->num << 1 | (firesat->subunit & 0x1); // ### ask IRM
		try_CMPEstablishPPconnection(firesat, firesat->subunit, firesat->isochannel);
		hpsb_listen_channel(&firesat_highlevel, firesat->host, firesat->isochannel);
		break;
	case FE_SLEEP:
		hpsb_unlisten_channel(&firesat_highlevel, firesat->host, firesat->isochannel);
		try_CMPBreakPPconnection(firesat, firesat->subunit, firesat->isochannel);
		firesat->isochannel = -1;
		break;
	case FE_GET_INFO:
//		printk(KERN_INFO "FE_GET_INFO\n");
		memcpy(arg, firesat->frontend_info,
		       sizeof (struct dvb_frontend_info));
		break;
	case FE_DISEQC_RESET_OVERLOAD:
	case FE_DISEQC_RECV_SLAVE_REPLY:
		return -EOPNOTSUPP;
	case FE_DISEQC_SEND_MASTER_CMD:
		return AVCLNBControl(firesat, LNBCONTROL_DONTCARE, LNBCONTROL_DONTCARE, LNBCONTROL_DONTCARE, 1, (struct dvb_diseqc_master_cmd *)arg);
		return 0;
	case FE_DISEQC_SEND_BURST:
		return 0;
//		return AVCLNBControl(firesat, LNBCONTROL_DONTCARE, (((fe_sec_mini_cmd_t)arg)==SEC_MINI_A)?0:1, LNBCONTROL_DONTCARE, 0, NULL);
	case FE_SET_TONE:
//		return AVCLNBControl(firesat, LNBCONTROL_DONTCARE, LNBCONTROL_DONTCARE, (((fe_sec_tone_mode_t) arg)==SEC_TONE_ON)?1:0, 0, NULL);
		firesat->tone = (fe_sec_tone_mode_t)arg;
		return 0;
	case FE_SET_VOLTAGE:
//		printk(KERN_INFO "FE_SET_VOLTAGE\n");
		firesat->voltage=(fe_sec_voltage_t) arg;
		return 0;
//		return AVCLNBControl(firesat, ((fe_sec_voltage_t) arg, LNBCONTROL_DONTCARE, LNBCONTROL_DONTCARE, 0, NULL);
	case FE_ENABLE_HIGH_LNB_VOLTAGE:
		return -EOPNOTSUPP;
	case FE_READ_STATUS: {
			ANTENNA_INPUT_INFO info;
			fe_status_t *status = (fe_status_t *)arg;
//			printk(KERN_INFO "%s: FE_READ_STATUS\n", __FUNCTION__);

			if(AVCTunerStatus(firesat,&info)) {
				return -EINVAL;
			}
//			printk(KERN_INFO "%s: NoRF=%c\n",__FUNCTION__,info.NoRF?'y':'n');
			if(info.NoRF)
				*status = 0;
			else
				*status = *status = FE_HAS_SIGNAL | FE_HAS_VITERBI |
				  FE_HAS_SYNC | FE_HAS_CARRIER | FE_HAS_LOCK;

			break;
		}
	case FE_READ_BER: {
			ANTENNA_INPUT_INFO info;
			u32 *ber = (u32 *)arg;
//			printk(KERN_INFO "%s: FE_READ_BER\n", __FUNCTION__);

			if(AVCTunerStatus(firesat,&info)) {
				return -EINVAL;
			}

//			*ber = *(u32*)info.BER; /* might need some byte ordering correction */
			*ber = ((info.BER[0] << 24) & 0xFF) | ((info.BER[1] << 16) & 0xFF) | ((info.BER[2] << 8) & 0xFF) | (info.BER[3] & 0xFF);

			break;
		}
	case FE_READ_SIGNAL_STRENGTH: {
			ANTENNA_INPUT_INFO info;
			u16 *signal = (u16 *)arg;
//			printk(KERN_INFO "%s: FE_READ_SIGNAL_STRENGTH\n", __FUNCTION__);

			if(AVCTunerStatus(firesat,&info)) {
				return -EINVAL;
			}

			*signal = info.SignalStrength;

			break;
		}
	case FE_READ_SNR:
	case FE_READ_UNCORRECTED_BLOCKS:
		return -EOPNOTSUPP;
	case FE_SET_FRONTEND: {
			struct dvb_frontend_parameters *p =
				(struct dvb_frontend_parameters *)arg;
//			printk(KERN_INFO "FE_GET_INFO\n");

//			printk(KERN_INFO "%s: FE_SET_FRONTEND\n", __FUNCTION__);

//			printk(KERN_INFO "            frequency->%d\n", p->frequency);
//			printk(KERN_INFO "            symbol_rate->%d\n",p->u.qam.symbol_rate);
//			printk(KERN_INFO "            inversion->%d\n", p->inversion);

//			if(AVCTuner_DSIT(firesat, 0/*###*/, p, NULL))
//				return -EINVAL;
			if(AVCTuner_DSD(firesat, p, NULL) != ACCEPTED)
				return -EINVAL;

			break;
		}
	case FE_GET_FRONTEND:
	case FE_GET_EVENT:
		return -EOPNOTSUPP;
	default:
		return -EINVAL;
	}

	return 0;
}

static struct firesat_channel *firesat_channel_allocate(struct firesat *firesat) {
	int k;

	printk(KERN_INFO "%s\n",__FUNCTION__);

	if(down_interruptible(&firesat->demux_sem))
		return NULL;

	for(k=0;k<16;k++) {
		printk(KERN_INFO "%s: channel %d: active = %d, pid = 0x%x\n",__FUNCTION__,k,firesat->channel[k].active,firesat->channel[k].pid);
		if(firesat->channel[k].active == 0) {
			firesat->channel[k].active = 1;
			up(&firesat->demux_sem);
			return &firesat->channel[k];
		}
	}

	up(&firesat->demux_sem);
	return NULL; // no more channels available
}

static int firesat_channel_collect(struct firesat *firesat, int *pidc, u16 pid[]) {
	int k, l=0;

	if(down_interruptible(&firesat->demux_sem))
		return -EINTR;

	for(k=0;k<16;k++)
		if(firesat->channel[k].active == 1)
			pid[l++] = firesat->channel[k].pid;

	up(&firesat->demux_sem);

	*pidc = l;

	return 0;
}

static int firesat_channel_release(struct firesat *firesat, struct firesat_channel *channel) {
	if(down_interruptible(&firesat->demux_sem))
		return -EINTR;

	channel->active = 0;

	up(&firesat->demux_sem);
	return 0;
}

static int firesat_start_feed(struct dvb_demux_feed *dvbdmxfeed) {
	struct firesat *firesat=(struct firesat*)dvbdmxfeed->demux->priv;
	struct firesat_channel *channel;
	int pidc,k;
	u16 pids[16];

	printk(KERN_INFO "%s (pid %u)\n",__FUNCTION__,dvbdmxfeed->pid);

	switch (dvbdmxfeed->type) {
	case DMX_TYPE_TS:
	case DMX_TYPE_SEC:
		break;
	default:
		printk("%s: invalid type %u\n",__FUNCTION__,dvbdmxfeed->type);
		return -EINVAL;
	}

	if (dvbdmxfeed->type == DMX_TYPE_TS) {
		switch (dvbdmxfeed->pes_type) {
		case DMX_TS_PES_VIDEO:
		case DMX_TS_PES_AUDIO:
		case DMX_TS_PES_TELETEXT:
		case DMX_TS_PES_PCR:
		case DMX_TS_PES_OTHER:
			channel = firesat_channel_allocate(firesat);
			break;
		default:
			printk("%s: invalid pes type %u\n",__FUNCTION__,dvbdmxfeed->pes_type);
			return -EINVAL;
		}
	} else {
		channel = firesat_channel_allocate(firesat);
	}

	if(!channel) {
		printk("%s: busy!\n",__FUNCTION__);
		return -EBUSY;
	}

	dvbdmxfeed->priv = channel;

	channel->dvbdmxfeed = dvbdmxfeed;
	channel->pid = dvbdmxfeed->pid;
	channel->type = dvbdmxfeed->type;
	channel->firesat = firesat;

	if(firesat_channel_collect(firesat, &pidc, pids)) {
		firesat_channel_release(firesat, channel);
		return -EINTR;
	}

	if((k=AVCTuner_SetPIDs(firesat, pidc, pids))) {
		firesat_channel_release(firesat, channel);
		printk("%s: AVCTuner failed with error %d\n",__FUNCTION__,k);
		return k;
	}

	return 0;
}

static int firesat_stop_feed(struct dvb_demux_feed *dvbdmxfeed) {
	struct dvb_demux *demux = dvbdmxfeed->demux;
	struct firesat *firesat=(struct firesat*)demux->priv;
	int k, l=0;
	u16 pids[16];

	printk(KERN_INFO "%s (pid %u)\n",__FUNCTION__,dvbdmxfeed->pid);

	if (dvbdmxfeed->type == DMX_TYPE_TS && !((dvbdmxfeed->ts_type & TS_PACKET) &&
				(demux->dmx.frontend->source != DMX_MEMORY_FE))) {
	   if (dvbdmxfeed->ts_type & TS_DECODER) {
		if (dvbdmxfeed->pes_type >= DMX_TS_PES_OTHER ||
			!demux->pesfilter[dvbdmxfeed->pes_type])
			return -EINVAL;
		demux->pids[dvbdmxfeed->pes_type] |= 0x8000;
		demux->pesfilter[dvbdmxfeed->pes_type] = 0;
	   }
	   if (!(dvbdmxfeed->ts_type & TS_DECODER &&
		dvbdmxfeed->pes_type < DMX_TS_PES_OTHER)) {
		return 0;
	   }
	}

	if(down_interruptible(&firesat->demux_sem))
		return -EINTR;

	// list except channel to be removed
	for(k=0;k<16;k++)
		if(firesat->channel[k].active == 1 && &firesat->channel[k] != (struct firesat_channel*)dvbdmxfeed->priv)
			pids[l++] = firesat->channel[k].pid;

	if((k=AVCTuner_SetPIDs(firesat, l, pids))) {
		up(&firesat->demux_sem);
		return k;
	}

	((struct firesat_channel*)dvbdmxfeed->priv)->active = 0;

	up(&firesat->demux_sem);

	return 0;
}

static int firesat_probe(struct device *dev) {
    struct unit_directory *ud=container_of(dev, struct unit_directory, device);
    struct firesat *firesat;
    unsigned long flags;
    int result;
	unsigned char subunitcount=0xff,subunit;
	struct firesat **firesats=kmalloc(sizeof(void*)*2,GFP_KERNEL);

	if(!firesats) {
		printk("%s: couldn't allocate memory.\n", __FUNCTION__);
		return -ENOMEM;
	}

//    printk(KERN_INFO "FireSAT: Detected device with GUID %08lx%04lx%04lx\n",(unsigned long)((ud->ne->guid)>>32),(unsigned long)(ud->ne->guid & 0xFFFF),(unsigned long)ud->ne->guid_vendor_id);
	printk(KERN_INFO "%s: loading device\n", __FUNCTION__);

	firesats[0]=NULL;
	firesats[1]=NULL;

	ud->device.driver_data = firesats;

    for(subunit=0;subunit<subunitcount;subunit++) {
		if (!(firesat = kmalloc(sizeof(struct firesat), GFP_KERNEL))) {
			printk("%s: couldn't allocate memory.\n", __FUNCTION__);
			kfree(firesats);
			return -ENOMEM;
		}

		memset(firesat, 0, sizeof(struct firesat));
		firesat->host=ud->ne->host;
		firesat->guid=ud->ne->guid;
		firesat->guid_vendor_id=ud->ne->guid_vendor_id;
		firesat->nodeentry=ud->ne;
		firesat->isochannel=-1;
		firesat->tone = 0xff;
		firesat->voltage = 0xff;
		if(!(firesat->respfrm = kmalloc(sizeof(AVCRspFrm), GFP_KERNEL))) {
			printk("%s: couldn't allocate memory.\n", __FUNCTION__);
			kfree(firesat);
			return -ENOMEM;
		}

		sema_init(&firesat->avc_sem, 1);
		atomic_set(&firesat->avc_reply_received, 1);
		sema_init(&firesat->demux_sem, 1);
		atomic_set(&firesat->reschedule_remotecontrol, 0);

		spin_lock_irqsave(&firesat_list_lock, flags);
		INIT_LIST_HEAD(&firesat->list);
		list_add_tail(&firesat->list, &firesat_list);
		spin_unlock_irqrestore(&firesat_list_lock, flags);

		if(subunit == 0) {
			firesat->subunit = 0x7; // 0x7 = don't care
			if(AVCSubUnitInfo(firesat, &subunitcount)) {
				printk("%s: AVC subunit info command failed.\n",__FUNCTION__);
				spin_lock_irqsave(&firesat_list_lock, flags);
				list_del(&firesat->list);
				spin_unlock_irqrestore(&firesat_list_lock, flags);
				kfree(firesat);
				return -EIO;
			}
		}

		printk(KERN_INFO "%s: subunit count = %d\n",__FUNCTION__,subunitcount);

		firesat->subunit=subunit;

		if(AVCIdentifySubunit(firesat,NULL,(int*)&firesat->type,&firesat->has_ci)) {
			printk("%s: cannot identify subunit %d\n",__FUNCTION__,subunit);
			spin_lock_irqsave(&firesat_list_lock, flags);
			list_del(&firesat->list);
			spin_unlock_irqrestore(&firesat_list_lock, flags);
			kfree(firesat);
			continue;
		}
		firesat->has_ci=1; // TEMP workaround

		switch(firesat->type) {
		case FireSAT_DVB_S:
			firesat->model_name="FireSAT DVB-S";
			firesat->frontend_info=&firesat_S_frontend_info;
			break;
		case FireSAT_DVB_C:
			firesat->model_name="FireSAT DVB-C";
			firesat->frontend_info=&firesat_C_frontend_info;
			break;
		case FireSAT_DVB_T:
			firesat->model_name="FireSAT DVB-T";
			firesat->frontend_info=&firesat_T_frontend_info;
			break;
		default:
			printk("%s: unknown model type 0x%x on subunit %d!\n",__FUNCTION__,firesat->type,subunit);
			firesat->model_name="Unknown";
			firesat->frontend_info=NULL;
		}
		if(!firesat->frontend_info) {
			spin_lock_irqsave(&firesat_list_lock, flags);
			list_del(&firesat->list);
			spin_unlock_irqrestore(&firesat_list_lock, flags);
			kfree(firesat);
			continue;
		}

		if ((result = dvb_register_adapter(&firesat->adapter,
										   firesat->model_name, THIS_MODULE)) < 0) {
			printk("%s: dvb_register_adapter failed: error %d\n",
				   __FUNCTION__, result);

			/* ### cleanup */
			spin_lock_irqsave(&firesat_list_lock, flags);
			list_del(&firesat->list);
			spin_unlock_irqrestore(&firesat_list_lock, flags);
			kfree(firesat);

			return result;
		}

		firesat->demux.dmx.capabilities = 0/*DMX_TS_FILTERING | DMX_SECTION_FILTERING*/;

		firesat->demux.priv = (void *)firesat;
		firesat->demux.filternum = 16;
		firesat->demux.feednum = 16;
		firesat->demux.start_feed = firesat_start_feed;
		firesat->demux.stop_feed = firesat_stop_feed;

		firesat->demux.write_to_decoder = NULL;

		if ((result = dvb_dmx_init(&firesat->demux)) < 0) {
			printk("%s: dvb_dmx_init failed: error %d\n", __FUNCTION__,
				   result);

			dvb_unregister_adapter(firesat->adapter);

			return result;
		}

		firesat->dmxdev.filternum = 16;
		firesat->dmxdev.demux = &firesat->demux.dmx;
		firesat->dmxdev.capabilities = 0;

		if ((result = dvb_dmxdev_init(&firesat->dmxdev, firesat->adapter)) < 0) {
			printk("%s: dvb_dmxdev_init failed: error %d\n",
				   __FUNCTION__, result);

			dvb_dmx_release(&firesat->demux);
			dvb_unregister_adapter(firesat->adapter);

			return result;
		}

		firesat->frontend.source = DMX_FRONTEND_0;

		if ((result = firesat->demux.dmx.add_frontend(&firesat->demux.dmx,
							  &firesat->frontend)) < 0) {
			printk("%s: dvb_dmx_init failed: error %d\n", __FUNCTION__,
				   result);

			dvb_dmxdev_release(&firesat->dmxdev);
			dvb_dmx_release(&firesat->demux);
			dvb_unregister_adapter(firesat->adapter);

			return result;
		}

		if ((result = firesat->demux.dmx.connect_frontend(&firesat->demux.dmx,
								  &firesat->frontend)) < 0) {
			printk("%s: dvb_dmx_init failed: error %d\n", __FUNCTION__,
				   result);

			firesat->demux.dmx.remove_frontend(&firesat->demux.dmx, &firesat->frontend);
			dvb_dmxdev_release(&firesat->dmxdev);
			dvb_dmx_release(&firesat->demux);
			dvb_unregister_adapter(firesat->adapter);

			return result;
		}

		dvb_net_init(firesat->adapter, &firesat->dvbnet, &firesat->demux.dmx);

		if((result= dvb_register_frontend(firesat_frontend_ioctl,firesat->adapter,firesat,firesat->frontend_info,THIS_MODULE)) < 0) {
			printk("%s: dvb_register_frontend_new failed: error %d\n", __FUNCTION__, result);
			/* ### cleanup */

			return result;
		}

		if(firesat->has_ci)
			firesat_ca_init(firesat);

		firesats[subunit] = firesat;
	} // loop fuer alle tuner am board

	if(firesats[0])
		AVCRegisterRemoteControl(firesats[0]);

    return 0;
}

static int firesat_remove(struct device *dev) {
    struct unit_directory *ud=container_of(dev, struct unit_directory, device);
    struct firesat **firesats=ud->device.driver_data;

    if(firesats) {
		int k;
		for(k=0;k<2;k++)
			if(firesats[k]) {
				unsigned long flags;
				if(firesats[k]->has_ci)
					firesat_ca_release(firesats[k]);
				dvb_unregister_frontend(firesat_frontend_ioctl,firesats[k]->adapter);

				dvb_net_release(&firesats[k]->dvbnet);
				firesats[k]->demux.dmx.close(&firesats[k]->demux.dmx);
				firesats[k]->demux.dmx.remove_frontend(&firesats[k]->demux.dmx, &firesats[k]->frontend);
				dvb_dmxdev_release(&firesats[k]->dmxdev);
				dvb_dmx_release(&firesats[k]->demux);
				dvb_unregister_adapter(firesats[k]->adapter);

				spin_lock_irqsave(&firesat_list_lock, flags);
				list_del(&firesats[k]->list);
				spin_unlock_irqrestore(&firesat_list_lock, flags);

				kfree(firesats[k]->respfrm);
				kfree(firesats[k]);
			}
		kfree(firesats);
	} else
		printk("%s: can't get firesat handle\n",__FUNCTION__);

    printk(KERN_INFO "FireSAT: Removing device with vendor id 0x%x, model id 0x%x.\n",ud->vendor_id,ud->model_id);
    return 0;
}

static int firesat_update(struct unit_directory *ud) {
//    printk(KERN_INFO "FireSAT: Found device with vendor id 0x%x, model id 0x%x\n", ud->vendor_id,ud->model_id);
    struct firesat **firesats=ud->device.driver_data;
    int k;
    // loop over subunits

	for(k=0;k<2;k++)
		if(firesats[k]) {
			firesats[k]->nodeentry=ud->ne;

			if(firesats[k]->isochannel >= 0)
				try_CMPEstablishPPconnection(firesats[k], firesats[k]->subunit, firesats[k]->isochannel);
		}

    return 0;
}

static struct hpsb_protocol_driver firesat_driver = {
	.name		= "FireSAT DVB",
	.id_table	= firesat_id_table,
    .update     = firesat_update,
	.driver         = {
		.name	= "firesat",
		.bus	= &ieee1394_bus_type,
	.probe  = firesat_probe,
	.remove = firesat_remove,
	},
};

static int __init firesat_init(void)
{
    int ret;
    printk("FireSAT loaded\n");
    hpsb_register_highlevel(&firesat_highlevel);
    ret = hpsb_register_protocol(&firesat_driver);
    if(ret) {
	printk(KERN_ERR "FireSAT: failed to register protocol\n");
	hpsb_unregister_highlevel(&firesat_highlevel);
	return ret;
    }

	if((ret=firesat_register_rc()))
		printk("%s: firesat_register_rc return error code %d (ignored)\n",__FUNCTION__,ret);

	return 0;
}

static void __exit firesat_exit(void){
	firesat_unregister_rc();

    hpsb_unregister_protocol(&firesat_driver);
    hpsb_unregister_highlevel(&firesat_highlevel);
    printk("FireSAT quit\n");
}

module_init(firesat_init);
module_exit(firesat_exit);

MODULE_AUTHOR("Andreas Monitzer <andy@monitzer.com>");
MODULE_DESCRIPTION("FireSAT DVB Driver");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("FireSAT DVB");
