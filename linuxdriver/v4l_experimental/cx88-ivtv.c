/*
 * $Id: cx88-ivtv.c,v 1.1 2005/11/30 17:26:37 mchehab Exp $
 *
 *  IVTV API emulation for the "blackbird" reference design.
 *
 *    (c) 2005 Catalin Climov <catalin@climov.com>
 *
 *  Includes parts from the ivtv driver( http://ivtv.sourceforge.net/),
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include "compat.h"
#include <media/v4l2-common.h>
#include "cx88.h"

MODULE_DESCRIPTION("ivtv ioctl emulation module for blackbird TV cards");
MODULE_AUTHOR("Catalin Climov <catalin@climov.com>");
MODULE_LICENSE("GPL");

static unsigned int debug = 0;
module_param(debug,int,0644);
MODULE_PARM_DESC(debug,"enable debug messages [blackbird]");

#define dprintk(level,fmt, arg...)	if (debug >= level) \
	printk(KERN_DEBUG "%s/2-bb: " fmt, dev->core->name , ## arg)

static int (*prev_ioctl_hook)(struct inode *inode, struct file *file,
			      unsigned int cmd, void *arg);
static unsigned int (*prev_ioctl_translator)(unsigned int cmd);

/* --- IVTV data structs -------------------------------------------- */

struct ivtv_ioctl_codec {
	uint32_t aspect;
	uint32_t audio_bitmask;
	uint32_t bframes;
	uint32_t bitrate_mode;
	uint32_t bitrate;
	uint32_t bitrate_peak;
	uint32_t dnr_mode;
	uint32_t dnr_spatial;
	uint32_t dnr_temporal;
	uint32_t dnr_type;
	uint32_t framerate; /* read only, ignored on write */
	uint32_t framespergop;  /* read only, ignored on write */
	uint32_t gop_closure;
	uint32_t pulldown;
	uint32_t stream_type;
};

struct ivtv_sliced_vbi_format {
	unsigned long service_set;  /* one or more of the IVTV_SLICED_ defines */
	unsigned long packet_size;  /* the size in bytes of the ivtv_sliced_data packet */
	unsigned long io_size;      /* maximum number of bytes passed by one read() call */
	unsigned long reserved;
};

#define IVTV_IOC_G_CODEC           _IOR ('@', 48, struct ivtv_ioctl_codec)
#define IVTV_IOC_S_CODEC           _IOW ('@', 49, struct ivtv_ioctl_codec)
#define IVTV_IOC_S_VBI_MODE        _IOWR('@', 35, struct ivtv_sliced_vbi_format)
#define IVTV_IOC_G_VBI_MODE        _IOR ('@', 36, struct ivtv_sliced_vbi_format)
#define IVTV_IOC_S_VBI_EMBED       _IOW ('@', 54, int)

/* ------------------------------------------------------------------ */

static unsigned int ivtv_translate_ioctl(unsigned int cmd)
{
#if 0
	printk( KERN_INFO "ivtv_translate_ioctl\n" );
#endif
	switch( cmd )
	{
		case 0xFFEE7703: cmd = IVTV_IOC_G_CODEC; break;
		case 0xFFEE7704: cmd = IVTV_IOC_S_CODEC; break;
		case 0xFFEE7781: /*cmd = IVTV_IOC_PLAY; break;*/
		case 0xFFEE7782: /*cmd = IVTV_IOC_PAUSE; break;*/
		case 0xFFEE7783: /*cmd = IVTV_IOC_FRAMESYNC; break;*/
		case 0xFFEE7784: /*cmd = IVTV_IOC_GET_TIMING; break;*/
		case 0xFFEE7785: /*cmd = IVTV_IOC_S_SLOW_FAST; break;*/
		case 0xFFEE7786: /*cmd = IVTV_IOC_S_START_DECODE; break;*/
		case 0xFFEE7787: /*cmd = IVTV_IOC_S_STOP_DECODE; break;*/
		case 0xFFEE7789: /*cmd = IVTV_IOC_GET_FB; break;*/
			printk( KERN_INFO "IVTV: 0x%x\n", cmd );
	}
	return prev_ioctl_translator( cmd );
}

static int ivtv_do_ioctl(struct inode *inode, struct file *file,
			 unsigned int cmd, void *arg)
{
	struct cx8802_fh  *fh  = file->private_data;
	struct cx8802_dev *dev = fh->dev;
	/* struct cx88_core  *core = dev->core; */

	/* int err; */

	if (debug > 1)
		v4l_print_ioctl(dev->core->name,cmd);
#if 1
	printk( KERN_INFO "IVTV IOCTL: 0x%x\n", cmd );
	v4l_print_ioctl(dev->core->name,cmd);
#endif
	dprintk( 1, "IVTV IOCTL: 0x%x\n", cmd );

	switch (cmd) {

	/* --- IVTV emulation layer --------------------------- */
	case IVTV_IOC_S_CODEC:
		printk( KERN_INFO "IVTV_IOC_S_CODEC\n" );
	case IVTV_IOC_G_CODEC:
	{
		struct ivtv_ioctl_codec *codec = arg;
#if 1
		printk( KERN_INFO "IVTV_IOC_G/S_CODEC\n" );
		printk( KERN_INFO "CODEC: aspect: %d\n", codec->aspect );
		printk( KERN_INFO "CODEC: audio : %d\n", codec->audio_bitmask );
		printk( KERN_INFO "CODEC: bfrms : %d\n", codec->bframes );
		printk( KERN_INFO "CODEC: br_mod: %d\n", codec->bitrate_mode );
		printk( KERN_INFO "CODEC: btrate: %d\n", codec->bitrate );
		printk( KERN_INFO "CODEC: btr_pk: %d\n", codec->bitrate_peak );
		printk( KERN_INFO "CODEC: dnr_md: %d\n", codec->dnr_mode );
		printk( KERN_INFO "CODEC: dnr_sp: %d\n", codec->dnr_spatial );
		printk( KERN_INFO "CODEC: dnr_tp: %d\n", codec->dnr_temporal );
		printk( KERN_INFO "CODEC: dnr_ty: %d\n", codec->dnr_type );
		printk( KERN_INFO "CODEC: framer: %d\n", codec->framerate );
		printk( KERN_INFO "CODEC: grmgop: %d\n", codec->framespergop );
		printk( KERN_INFO "CODEC: gop_cl: %d\n", codec->gop_closure );
		printk( KERN_INFO "CODEC: pulldn: %d\n", codec->pulldown );
		printk( KERN_INFO "CODEC: strtyp: %d\n\n", codec->stream_type );
#endif

		codec->aspect = 2; /* 4:3 */
		codec->audio_bitmask = (2 << 2) | (14 << 4); /* layer II | 384kbps */
		codec->bframes = 2;
		codec->bitrate_mode = 1; /* cbr */
		codec->bitrate = 4500000; /* bps */
		codec->bitrate_peak = 6000000; /* peak */
		codec->dnr_mode = 0; /* spatial=manual | temporal=manual */
		codec->dnr_spatial = 0;
		codec->dnr_temporal = 0;
		codec->dnr_type = 0; /* disabled */
		codec->framerate = 1; /* 25fps */
		codec->framespergop = 15;
		codec->gop_closure = 0; /* open */
		codec->pulldown = 0; /* enabled */
		codec->stream_type = 0; /* program stream */
#if 1
		printk( KERN_INFO "CODEC: aspect: %d\n", codec->aspect );
		printk( KERN_INFO "CODEC: audio : %d\n", codec->audio_bitmask );
		printk( KERN_INFO "CODEC: bfrms : %d\n", codec->bframes );
		printk( KERN_INFO "CODEC: br_mod: %d\n", codec->bitrate_mode );
		printk( KERN_INFO "CODEC: btrate: %d\n", codec->bitrate );
		printk( KERN_INFO "CODEC: btr_pk: %d\n", codec->bitrate_peak );
		printk( KERN_INFO "CODEC: dnr_md: %d\n", codec->dnr_mode );
		printk( KERN_INFO "CODEC: dnr_sp: %d\n", codec->dnr_spatial );
		printk( KERN_INFO "CODEC: dnr_tp: %d\n", codec->dnr_temporal );
		printk( KERN_INFO "CODEC: dnr_ty: %d\n", codec->dnr_type );
		printk( KERN_INFO "CODEC: framer: %d\n", codec->framerate );
		printk( KERN_INFO "CODEC: grmgop: %d\n", codec->framespergop );
		printk( KERN_INFO "CODEC: gop_cl: %d\n", codec->gop_closure );
		printk( KERN_INFO "CODEC: pulldn: %d\n", codec->pulldown );
		printk( KERN_INFO "CODEC: strtyp: %d\n", codec->stream_type );
#endif
		return 0;
	}
	case IVTV_IOC_S_VBI_MODE:
	{
		struct ivtv_sliced_vbi_format *fmt = arg;
		printk( KERN_INFO "IVTV_IOC_S_VBI_MODE: ss: %ld, ps: %ld, is: %ld\n",
			fmt->service_set, fmt->packet_size, fmt->io_size );
#if 0
		fmt->service_set = 0;  /* one or more of the IVTV_SLICED_ defines */
		fmt->packet_size = 0;  /* the size in bytes of the ivtv_sliced_data packet */
		fmt->io_size = 0;      /* maximum number of bytes passed by one read() call */
#endif
		return 0;
	}
	case IVTV_IOC_G_VBI_MODE:
	{
		struct ivtv_sliced_vbi_format *fmt = arg;

		fmt->service_set = 1;  /* one or more of the IVTV_SLICED_ defines */
		fmt->packet_size = 0;  /* the size in bytes of the ivtv_sliced_data packet */
		fmt->io_size = 0;      /* maximum number of bytes passed by one read() call */
		return 0;
	}
	case IVTV_IOC_S_VBI_EMBED:
	{
		int *embed = arg;
		printk( KERN_INFO "IVTV_IOC_S_VBI_EMBED: %d\n", *embed );
			return 0;
	}

	default:
		if( prev_ioctl_hook )
			return prev_ioctl_hook( inode, file, cmd, arg );
		else
			return -EINVAL;
	}
	return 0;
}

static int __init cx88_ivtv_init(void)
{
	printk(KERN_INFO "ivtv emulation for blackbird version %d.%d.%d loaded\n",
		(CX88_VERSION_CODE >> 16) & 0xff,
		(CX88_VERSION_CODE >>  8) & 0xff,
		CX88_VERSION_CODE & 0xff);
#ifdef SNAPSHOT
	printk(KERN_INFO "cx2388x: snapshot date %04d-%02d-%02d\n",
		SNAPSHOT/10000, (SNAPSHOT/100)%100, SNAPSHOT%100);
#endif
	request_module( "cx88-blackbird" );
	prev_ioctl_translator = cx88_ioctl_translator;
	cx88_ioctl_translator = ivtv_translate_ioctl;
	prev_ioctl_hook = cx88_ioctl_hook;
	cx88_ioctl_hook = ivtv_do_ioctl;
	return 0;
}

static void __exit cx88_ivtv_fini(void)
{
	cx88_ioctl_hook = prev_ioctl_hook;
	cx88_ioctl_translator = prev_ioctl_translator;
}

module_init(cx88_ivtv_init);
module_exit(cx88_ivtv_fini);

/*
 * kate: eol "unix"; indent-width 3; remove-trailing-space on; replace-trailing-space-save on; tab-width 8; replace-tabs off; space-indent off; mixed-indent off
 */
