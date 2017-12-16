/*
 *	Video for Linux Two
 *
 *	A generic video device interface for the LINUX operating system
 *	using a set of device structures/vectors for low level operations.
 *
 *	This file replaces the videodev.c file that comes with the
 *	regular kernel distribution.
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 * Author:	Bill Dirks <bdirks@pacbell.net>
 *		based on code by Alan Cox, <alan@cymru.net>
 *
 */

/*
 * Video capture interface for Linux
 *
 *	A generic video device interface for the LINUX operating system
 *	using a set of device structures/vectors for low level operations.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Author:	Alan Cox, <alan@redhat.com>
 *
 * Fixes:
 */

/*
 * Video4linux 1/2 integration by Justin Schoeman
 * <justin@suntiger.ee.up.ac.za>
 * 2.4 PROCFS support ported from 2.4 kernels by
 *  I�aki Garc�a Etxebarria <garetxe@euskalnet.net>
 * Makefile fix by "W. Michael Petullo" <mike@flyn.org>
 * 2.4 devfs support ported from 2.4 kernels by
 *  Dan Merillat <dan@merillat.org>
 * Added Gerd Knorrs v4l1 enhancements (Justin Schoeman)
 */

#include "compat.h"
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/io.h>
#include <asm/div64.h>
#include <linux/video_decoder.h>
#define __OLD_VIDIOC_ /* To allow fixing old calls*/
#include <media/v4l2-common.h>

#ifdef CONFIG_KMOD
#include <linux/kmod.h>
#endif

#if defined(CONFIG_UST) || defined(CONFIG_UST_MODULE)
#include <linux/ust.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
# define strlcpy(dest,src,len) strncpy(dest,src,(len)-1)
#endif

#include <linux/videodev.h>

MODULE_AUTHOR("Bill Dirks, Justin Schoeman, Gerd Knorr");
MODULE_DESCRIPTION("misc helper functions for v4l2 device drivers");
MODULE_LICENSE("GPL");

/*
 *
 *	V 4 L 2   D R I V E R   H E L P E R   A P I
 *
 */

/*
 *  Video Standard Operations (contributed by Michael Schimek)
 */

#if 0 /* seems to have no users */
/* This is the recommended method to deal with the framerate fields. More
   sophisticated drivers will access the fields directly. */
unsigned int
v4l2_video_std_fps(struct v4l2_standard *vs)
{
	if (vs->frameperiod.numerator > 0)
		return (((vs->frameperiod.denominator << 8) /
			 vs->frameperiod.numerator) +
			(1 << 7)) / (1 << 8);
	return 0;
}
EXPORT_SYMBOL(v4l2_video_std_fps);
#endif

/* Fill in the fields of a v4l2_standard structure according to the
   'id' and 'transmission' parameters.  Returns negative on error.  */
int v4l2_video_std_construct(struct v4l2_standard *vs,
			     int id, char *name)
{
	u32 index = vs->index;

	memset(vs, 0, sizeof(struct v4l2_standard));
	vs->index = index;
	vs->id    = id;
	if (id & V4L2_STD_525_60) {
		vs->frameperiod.numerator = 1001;
		vs->frameperiod.denominator = 30000;
		vs->framelines = 525;
	} else {
		vs->frameperiod.numerator = 1;
		vs->frameperiod.denominator = 25;
		vs->framelines = 625;
	}
	strlcpy(vs->name,name,sizeof(vs->name));
	return 0;
}

/* ----------------------------------------------------------------- */
/* priority handling                                                 */

#define V4L2_PRIO_VALID(val) (val == V4L2_PRIORITY_BACKGROUND   || \
			      val == V4L2_PRIORITY_INTERACTIVE  || \
			      val == V4L2_PRIORITY_RECORD)

int v4l2_prio_init(struct v4l2_prio_state *global)
{
	memset(global,0,sizeof(*global));
	return 0;
}

int v4l2_prio_change(struct v4l2_prio_state *global, enum v4l2_priority *local,
		     enum v4l2_priority new)
{
	if (!V4L2_PRIO_VALID(new))
		return -EINVAL;
	if (*local == new)
		return 0;

	atomic_inc(&global->prios[new]);
	if (V4L2_PRIO_VALID(*local))
		atomic_dec(&global->prios[*local]);
	*local = new;
	return 0;
}

int v4l2_prio_open(struct v4l2_prio_state *global, enum v4l2_priority *local)
{
	return v4l2_prio_change(global,local,V4L2_PRIORITY_DEFAULT);
}

int v4l2_prio_close(struct v4l2_prio_state *global, enum v4l2_priority *local)
{
	if (V4L2_PRIO_VALID(*local))
		atomic_dec(&global->prios[*local]);
	return 0;
}

enum v4l2_priority v4l2_prio_max(struct v4l2_prio_state *global)
{
	if (atomic_read(&global->prios[V4L2_PRIORITY_RECORD]) > 0)
		return V4L2_PRIORITY_RECORD;
	if (atomic_read(&global->prios[V4L2_PRIORITY_INTERACTIVE]) > 0)
		return V4L2_PRIORITY_INTERACTIVE;
	if (atomic_read(&global->prios[V4L2_PRIORITY_BACKGROUND]) > 0)
		return V4L2_PRIORITY_BACKGROUND;
	return V4L2_PRIORITY_UNSET;
}

int v4l2_prio_check(struct v4l2_prio_state *global, enum v4l2_priority *local)
{
	if (*local < v4l2_prio_max(global))
		return -EBUSY;
	return 0;
}


/* ----------------------------------------------------------------- */
/* some arrays for pretty-printing debug messages of enum types      */

char *v4l2_field_names[] = {
	[V4L2_FIELD_ANY]        = "any",
	[V4L2_FIELD_NONE]       = "none",
	[V4L2_FIELD_TOP]        = "top",
	[V4L2_FIELD_BOTTOM]     = "bottom",
	[V4L2_FIELD_INTERLACED] = "interlaced",
	[V4L2_FIELD_SEQ_TB]     = "seq-tb",
	[V4L2_FIELD_SEQ_BT]     = "seq-bt",
	[V4L2_FIELD_ALTERNATE]  = "alternate",
};

char *v4l2_type_names[] = {
	[V4L2_BUF_TYPE_VIDEO_CAPTURE] = "video-cap",
	[V4L2_BUF_TYPE_VIDEO_OVERLAY] = "video-over",
	[V4L2_BUF_TYPE_VIDEO_OUTPUT]  = "video-out",
	[V4L2_BUF_TYPE_VBI_CAPTURE]   = "vbi-cap",
	[V4L2_BUF_TYPE_VBI_OUTPUT]    = "vbi-out",
};

static char *v4l2_memory_names[] = {
	[V4L2_MEMORY_MMAP]    = "mmap",
	[V4L2_MEMORY_USERPTR] = "userptr",
	[V4L2_MEMORY_OVERLAY] = "overlay",
};

#define prt_names(a,arr) (((a)>=0)&&((a)<ARRAY_SIZE(arr)))?arr[a]:"unknown"

/* ------------------------------------------------------------------ */
/* debug help functions                                               */

#ifdef CONFIG_VIDEO_V4L1_COMPAT
static const char *v4l1_ioctls[] = {
	[_IOC_NR(VIDIOCGCAP)]       = "VIDIOCGCAP",
	[_IOC_NR(VIDIOCGCHAN)]      = "VIDIOCGCHAN",
	[_IOC_NR(VIDIOCSCHAN)]      = "VIDIOCSCHAN",
	[_IOC_NR(VIDIOCGTUNER)]     = "VIDIOCGTUNER",
	[_IOC_NR(VIDIOCSTUNER)]     = "VIDIOCSTUNER",
	[_IOC_NR(VIDIOCGPICT)]      = "VIDIOCGPICT",
	[_IOC_NR(VIDIOCSPICT)]      = "VIDIOCSPICT",
	[_IOC_NR(VIDIOCCAPTURE)]    = "VIDIOCCAPTURE",
	[_IOC_NR(VIDIOCGWIN)]       = "VIDIOCGWIN",
	[_IOC_NR(VIDIOCSWIN)]       = "VIDIOCSWIN",
	[_IOC_NR(VIDIOCGFBUF)]      = "VIDIOCGFBUF",
	[_IOC_NR(VIDIOCSFBUF)]      = "VIDIOCSFBUF",
	[_IOC_NR(VIDIOCKEY)]        = "VIDIOCKEY",
	[_IOC_NR(VIDIOCGFREQ)]      = "VIDIOCGFREQ",
	[_IOC_NR(VIDIOCSFREQ)]      = "VIDIOCSFREQ",
	[_IOC_NR(VIDIOCGAUDIO)]     = "VIDIOCGAUDIO",
	[_IOC_NR(VIDIOCSAUDIO)]     = "VIDIOCSAUDIO",
	[_IOC_NR(VIDIOCSYNC)]       = "VIDIOCSYNC",
	[_IOC_NR(VIDIOCMCAPTURE)]   = "VIDIOCMCAPTURE",
	[_IOC_NR(VIDIOCGMBUF)]      = "VIDIOCGMBUF",
	[_IOC_NR(VIDIOCGUNIT)]      = "VIDIOCGUNIT",
	[_IOC_NR(VIDIOCGCAPTURE)]   = "VIDIOCGCAPTURE",
	[_IOC_NR(VIDIOCSCAPTURE)]   = "VIDIOCSCAPTURE",
	[_IOC_NR(VIDIOCSPLAYMODE)]  = "VIDIOCSPLAYMODE",
	[_IOC_NR(VIDIOCSWRITEMODE)] = "VIDIOCSWRITEMODE",
	[_IOC_NR(VIDIOCGPLAYINFO)]  = "VIDIOCGPLAYINFO",
	[_IOC_NR(VIDIOCSMICROCODE)] = "VIDIOCSMICROCODE",
	[_IOC_NR(VIDIOCGVBIFMT)]    = "VIDIOCGVBIFMT",
	[_IOC_NR(VIDIOCSVBIFMT)]    = "VIDIOCSVBIFMT"
};
#define V4L1_IOCTLS ARRAY_SIZE(v4l1_ioctls)
#endif

static const char *v4l2_ioctls[] = {
	[_IOC_NR(VIDIOC_QUERYCAP)]         = "VIDIOC_QUERYCAP",
	[_IOC_NR(VIDIOC_RESERVED)]         = "VIDIOC_RESERVED",
	[_IOC_NR(VIDIOC_ENUM_FMT)]         = "VIDIOC_ENUM_FMT",
	[_IOC_NR(VIDIOC_G_FMT)]            = "VIDIOC_G_FMT",
	[_IOC_NR(VIDIOC_S_FMT)]            = "VIDIOC_S_FMT",
#if 1 /* experimental*/
	[_IOC_NR(VIDIOC_G_MPEGCOMP)]       = "VIDIOC_G_MPEGCOMP",
	[_IOC_NR(VIDIOC_S_MPEGCOMP)]       = "VIDIOC_S_MPEGCOMP",
#endif
	[_IOC_NR(VIDIOC_REQBUFS)]          = "VIDIOC_REQBUFS",
	[_IOC_NR(VIDIOC_QUERYBUF)]         = "VIDIOC_QUERYBUF",
	[_IOC_NR(VIDIOC_G_FBUF)]           = "VIDIOC_G_FBUF",
	[_IOC_NR(VIDIOC_S_FBUF)]           = "VIDIOC_S_FBUF",
	[_IOC_NR(VIDIOC_OVERLAY)]          = "VIDIOC_OVERLAY",
	[_IOC_NR(VIDIOC_QBUF)]             = "VIDIOC_QBUF",
	[_IOC_NR(VIDIOC_DQBUF)]            = "VIDIOC_DQBUF",
	[_IOC_NR(VIDIOC_STREAMON)]         = "VIDIOC_STREAMON",
	[_IOC_NR(VIDIOC_STREAMOFF)]        = "VIDIOC_STREAMOFF",
	[_IOC_NR(VIDIOC_G_PARM)]           = "VIDIOC_G_PARM",
	[_IOC_NR(VIDIOC_S_PARM)]           = "VIDIOC_S_PARM",
	[_IOC_NR(VIDIOC_G_STD)]            = "VIDIOC_G_STD",
	[_IOC_NR(VIDIOC_S_STD)]            = "VIDIOC_S_STD",
	[_IOC_NR(VIDIOC_ENUMSTD)]          = "VIDIOC_ENUMSTD",
	[_IOC_NR(VIDIOC_ENUMINPUT)]        = "VIDIOC_ENUMINPUT",
	[_IOC_NR(VIDIOC_G_CTRL)]           = "VIDIOC_G_CTRL",
	[_IOC_NR(VIDIOC_S_CTRL)]           = "VIDIOC_S_CTRL",
	[_IOC_NR(VIDIOC_G_TUNER)]          = "VIDIOC_G_TUNER",
	[_IOC_NR(VIDIOC_S_TUNER)]          = "VIDIOC_S_TUNER",
	[_IOC_NR(VIDIOC_G_AUDIO)]          = "VIDIOC_G_AUDIO",
	[_IOC_NR(VIDIOC_S_AUDIO)]          = "VIDIOC_S_AUDIO",
	[_IOC_NR(VIDIOC_QUERYCTRL)]        = "VIDIOC_QUERYCTRL",
	[_IOC_NR(VIDIOC_QUERYMENU)]        = "VIDIOC_QUERYMENU",
	[_IOC_NR(VIDIOC_G_INPUT)]          = "VIDIOC_G_INPUT",
	[_IOC_NR(VIDIOC_S_INPUT)]          = "VIDIOC_S_INPUT",
	[_IOC_NR(VIDIOC_G_OUTPUT)]         = "VIDIOC_G_OUTPUT",
	[_IOC_NR(VIDIOC_S_OUTPUT)]         = "VIDIOC_S_OUTPUT",
	[_IOC_NR(VIDIOC_ENUMOUTPUT)]       = "VIDIOC_ENUMOUTPUT",
	[_IOC_NR(VIDIOC_G_AUDOUT)]         = "VIDIOC_G_AUDOUT",
	[_IOC_NR(VIDIOC_S_AUDOUT)]         = "VIDIOC_S_AUDOUT",
	[_IOC_NR(VIDIOC_G_MODULATOR)]      = "VIDIOC_G_MODULATOR",
	[_IOC_NR(VIDIOC_S_MODULATOR)]      = "VIDIOC_S_MODULATOR",
	[_IOC_NR(VIDIOC_G_FREQUENCY)]      = "VIDIOC_G_FREQUENCY",
	[_IOC_NR(VIDIOC_S_FREQUENCY)]      = "VIDIOC_S_FREQUENCY",
	[_IOC_NR(VIDIOC_CROPCAP)]          = "VIDIOC_CROPCAP",
	[_IOC_NR(VIDIOC_G_CROP)]           = "VIDIOC_G_CROP",
	[_IOC_NR(VIDIOC_S_CROP)]           = "VIDIOC_S_CROP",
	[_IOC_NR(VIDIOC_G_JPEGCOMP)]       = "VIDIOC_G_JPEGCOMP",
	[_IOC_NR(VIDIOC_S_JPEGCOMP)]       = "VIDIOC_S_JPEGCOMP",
	[_IOC_NR(VIDIOC_QUERYSTD)]         = "VIDIOC_QUERYSTD",
	[_IOC_NR(VIDIOC_TRY_FMT)]          = "VIDIOC_TRY_FMT",
	[_IOC_NR(VIDIOC_ENUMAUDIO)]        = "VIDIOC_ENUMAUDIO",
	[_IOC_NR(VIDIOC_ENUMAUDOUT)]       = "VIDIOC_ENUMAUDOUT",
	[_IOC_NR(VIDIOC_G_PRIORITY)]       = "VIDIOC_G_PRIORITY",
	[_IOC_NR(VIDIOC_S_PRIORITY)]       = "VIDIOC_S_PRIORITY",
#if 1 /*KEEP*/
	[_IOC_NR(VIDIOC_G_SLICED_VBI_CAP)] = "VIDIOC_G_SLICED_VBI_CAP",
#endif
	[_IOC_NR(VIDIOC_LOG_STATUS)]       = "VIDIOC_LOG_STATUS",
	[_IOC_NR(VIDIOC_G_EXT_CTRLS)]      = "VIDIOC_G_EXT_CTRLS",
	[_IOC_NR(VIDIOC_S_EXT_CTRLS)]      = "VIDIOC_S_EXT_CTRLS",
	[_IOC_NR(VIDIOC_TRY_EXT_CTRLS)]    = "VIDIOC_TRY_EXT_CTRLS"
};
#define V4L2_IOCTLS ARRAY_SIZE(v4l2_ioctls)

static const char *v4l2_int_ioctls[] = {
#ifdef CONFIG_VIDEO_V4L1_COMPAT
	[_IOC_NR(DECODER_GET_CAPABILITIES)]    = "DECODER_GET_CAPABILITIES",
	[_IOC_NR(DECODER_GET_STATUS)]          = "DECODER_GET_STATUS",
	[_IOC_NR(DECODER_SET_NORM)]            = "DECODER_SET_NORM",
	[_IOC_NR(DECODER_SET_INPUT)]           = "DECODER_SET_INPUT",
	[_IOC_NR(DECODER_SET_OUTPUT)]          = "DECODER_SET_OUTPUT",
	[_IOC_NR(DECODER_ENABLE_OUTPUT)]       = "DECODER_ENABLE_OUTPUT",
	[_IOC_NR(DECODER_SET_PICTURE)]         = "DECODER_SET_PICTURE",
	[_IOC_NR(DECODER_SET_GPIO)]            = "DECODER_SET_GPIO",
	[_IOC_NR(DECODER_INIT)]                = "DECODER_INIT",
	[_IOC_NR(DECODER_SET_VBI_BYPASS)]      = "DECODER_SET_VBI_BYPASS",
	[_IOC_NR(DECODER_DUMP)]                = "DECODER_DUMP",
#endif
	[_IOC_NR(AUDC_SET_RADIO)]              = "AUDC_SET_RADIO",

	[_IOC_NR(TUNER_SET_TYPE_ADDR)]         = "TUNER_SET_TYPE_ADDR",
	[_IOC_NR(TUNER_SET_STANDBY)]           = "TUNER_SET_STANDBY",
	[_IOC_NR(TDA9887_SET_CONFIG)]          = "TDA9887_SET_CONFIG",

	[_IOC_NR(VIDIOC_INT_S_TUNER_MODE)]     = "VIDIOC_INT_S_TUNER_MODE",
	[_IOC_NR(VIDIOC_INT_S_REGISTER)]       = "VIDIOC_INT_S_REGISTER",
	[_IOC_NR(VIDIOC_INT_G_REGISTER)]       = "VIDIOC_INT_G_REGISTER",
	[_IOC_NR(VIDIOC_INT_RESET)]            = "VIDIOC_INT_RESET",
	[_IOC_NR(VIDIOC_INT_AUDIO_CLOCK_FREQ)] = "VIDIOC_INT_AUDIO_CLOCK_FREQ",
	[_IOC_NR(VIDIOC_INT_DECODE_VBI_LINE)]  = "VIDIOC_INT_DECODE_VBI_LINE",
	[_IOC_NR(VIDIOC_INT_S_VBI_DATA)]       = "VIDIOC_INT_S_VBI_DATA",
	[_IOC_NR(VIDIOC_INT_G_VBI_DATA)]       = "VIDIOC_INT_G_VBI_DATA",
	[_IOC_NR(VIDIOC_INT_G_CHIP_IDENT)]     = "VIDIOC_INT_G_CHIP_IDENT",
	[_IOC_NR(VIDIOC_INT_I2S_CLOCK_FREQ)]   = "VIDIOC_INT_I2S_CLOCK_FREQ",
	[_IOC_NR(VIDIOC_INT_S_STANDBY)]        = "VIDIOC_INT_S_STANDBY",
	[_IOC_NR(VIDIOC_INT_S_AUDIO_ROUTING)]  = "VIDIOC_INT_S_AUDIO_ROUTING",
	[_IOC_NR(VIDIOC_INT_G_AUDIO_ROUTING)]  = "VIDIOC_INT_G_AUDIO_ROUTING",
	[_IOC_NR(VIDIOC_INT_S_VIDEO_ROUTING)]  = "VIDIOC_INT_S_VIDEO_ROUTING",
	[_IOC_NR(VIDIOC_INT_G_VIDEO_ROUTING)]  = "VIDIOC_INT_G_VIDEO_ROUTING",
	[_IOC_NR(VIDIOC_INT_S_CRYSTAL_FREQ)]   = "VIDIOC_INT_S_CRYSTAL_FREQ"
};
#define V4L2_INT_IOCTLS ARRAY_SIZE(v4l2_int_ioctls)

static void v4l_print_pix_fmt (char *s, struct v4l2_pix_format *fmt)
{
	printk ("%s: width=%d, height=%d, format=%d, field=%s, "
		"bytesperline=%d sizeimage=%d, colorspace=%d\n", s,
		fmt->width,fmt->height,fmt->pixelformat,
		prt_names(fmt->field,v4l2_field_names),
		fmt->bytesperline,fmt->sizeimage,fmt->colorspace);
};

/* Common ioctl debug function. This function can be used by
   external ioctl messages as well as internal V4L ioctl */
void v4l_printk_ioctl(unsigned int cmd)
{
	char *dir;

	switch (_IOC_DIR(cmd)) {
	case _IOC_NONE:              dir = "--"; break;
	case _IOC_READ:              dir = "r-"; break;
	case _IOC_WRITE:             dir = "-w"; break;
	case _IOC_READ | _IOC_WRITE: dir = "rw"; break;
	default:                     dir = "*ERR*"; break;
	}
	switch (_IOC_TYPE(cmd)) {
	case 'd':
		printk("v4l2_int ioctl %s, dir=%s (0x%08x)\n",
		       (_IOC_NR(cmd) < V4L2_INT_IOCTLS) ?
		       v4l2_int_ioctls[_IOC_NR(cmd)] : "UNKNOWN", dir, cmd);
		break;
#ifdef CONFIG_VIDEO_V4L1_COMPAT
	case 'v':
		printk("v4l1 ioctl %s, dir=%s (0x%08x)\n",
		       (_IOC_NR(cmd) < V4L1_IOCTLS) ?
		       v4l1_ioctls[_IOC_NR(cmd)] : "UNKNOWN", dir, cmd);
		break;
#endif
	case 'V':
		printk("v4l2 ioctl %s, dir=%s (0x%08x)\n",
		       (_IOC_NR(cmd) < V4L2_IOCTLS) ?
		       v4l2_ioctls[_IOC_NR(cmd)] : "UNKNOWN", dir, cmd);
		break;

	default:
		printk("unknown ioctl '%c', dir=%s, #%d (0x%08x)\n",
		       _IOC_TYPE(cmd), dir, _IOC_NR(cmd), cmd);
	}
}

/* Common ioctl debug function. This function can be used by
   external ioctl messages as well as internal V4L ioctl and its
   arguments */
void v4l_printk_ioctl_arg(char *s,unsigned int cmd, void *arg)
{
	printk(s);
	printk(": ");
	v4l_printk_ioctl(cmd);
	switch (cmd) {
	case VIDIOC_INT_G_CHIP_IDENT:
	{
		enum v4l2_chip_ident  *p=arg;
		printk ("%s: chip ident=%d\n", s, *p);
		break;
	}
	case VIDIOC_G_PRIORITY:
	case VIDIOC_S_PRIORITY:
	{
		enum v4l2_priority *p=arg;
		printk ("%s: priority=%d\n", s, *p);
		break;
	}
	case VIDIOC_INT_S_TUNER_MODE:
	{
		enum v4l2_tuner_type *p=arg;
		printk ("%s: tuner type=%d\n", s, *p);
		break;
	}
#ifdef CONFIG_VIDEO_V4L1_COMPAT
	case DECODER_SET_VBI_BYPASS:
	case DECODER_ENABLE_OUTPUT:
	case DECODER_GET_STATUS:
	case DECODER_SET_OUTPUT:
	case DECODER_SET_INPUT:
	case DECODER_SET_GPIO:
	case DECODER_SET_NORM:
	case VIDIOCCAPTURE:
	case VIDIOCSYNC:
	case VIDIOCSWRITEMODE:
#endif
	case TUNER_SET_TYPE_ADDR:
	case TUNER_SET_STANDBY:
	case TDA9887_SET_CONFIG:
#ifdef __OLD_VIDIOC_
	case VIDIOC_OVERLAY_OLD:
#endif
	case VIDIOC_STREAMOFF:
	case VIDIOC_G_OUTPUT:
	case VIDIOC_S_OUTPUT:
	case VIDIOC_STREAMON:
	case VIDIOC_G_INPUT:
	case VIDIOC_OVERLAY:
	case VIDIOC_S_INPUT:
	{
		int *p=arg;
		printk ("%s: value=%d\n", s, *p);
		break;
	}
	case VIDIOC_G_AUDIO:
	case VIDIOC_S_AUDIO:
	case VIDIOC_ENUMAUDIO:
#ifdef __OLD_VIDIOC_
	case VIDIOC_G_AUDIO_OLD:
#endif
	{
		struct v4l2_audio *p=arg;

		printk ("%s: index=%d, name=%s, capability=%d, mode=%d\n",
			s,p->index, p->name,p->capability, p->mode);
		break;
	}
	case VIDIOC_G_AUDOUT:
	case VIDIOC_S_AUDOUT:
	case VIDIOC_ENUMAUDOUT:
#ifdef __OLD_VIDIOC_
	case VIDIOC_G_AUDOUT_OLD:
#endif
	{
		struct v4l2_audioout *p=arg;
		printk ("%s: index=%d, name=%s, capability=%d, mode=%d\n", s,
				p->index, p->name, p->capability,p->mode);
		break;
	}
	case VIDIOC_QBUF:
	case VIDIOC_DQBUF:
	case VIDIOC_QUERYBUF:
	{
		struct v4l2_buffer *p=arg;
		struct v4l2_timecode *tc=&p->timecode;
		printk ("%s: %02ld:%02d:%02d.%08ld index=%d, type=%s, "
			"bytesused=%d, flags=0x%08x, "
			"field=%0d, sequence=%d, memory=%s, offset/userptr=0x%08lx\n",
				s,
				(p->timestamp.tv_sec/3600),
				(int)(p->timestamp.tv_sec/60)%60,
				(int)(p->timestamp.tv_sec%60),
				p->timestamp.tv_usec,
				p->index,
				prt_names(p->type,v4l2_type_names),
				p->bytesused,p->flags,
				p->field,p->sequence,
				prt_names(p->memory,v4l2_memory_names),
				p->m.userptr);
		printk ("%s: timecode= %02d:%02d:%02d type=%d, "
			"flags=0x%08x, frames=%d, userbits=0x%08x\n",
				s,tc->hours,tc->minutes,tc->seconds,
				tc->type, tc->flags, tc->frames, *(__u32 *) tc->userbits);
		break;
	}
	case VIDIOC_QUERYCAP:
	{
		struct v4l2_capability *p=arg;
		printk ("%s: driver=%s, card=%s, bus=%s, version=0x%08x, "
			"capabilities=0x%08x\n", s,
				p->driver,p->card,p->bus_info,
				p->version,
				p->capabilities);
		break;
	}
	case VIDIOC_G_CTRL:
	case VIDIOC_S_CTRL:
#ifdef __OLD_VIDIOC_
	case VIDIOC_S_CTRL_OLD:
#endif
	{
		struct v4l2_control *p=arg;
		printk ("%s: id=%d, value=%d\n", s, p->id, p->value);
		break;
	}
	case VIDIOC_G_EXT_CTRLS:
	case VIDIOC_S_EXT_CTRLS:
	case VIDIOC_TRY_EXT_CTRLS:
	{
		struct v4l2_ext_controls *p = arg;
		int i;

		printk("%s: ctrl_class=%d, count=%d\n", s, p->ctrl_class, p->count);
		for (i = 0; i < p->count; i++) {
			struct v4l2_ext_control *c = &p->controls[i];
			if (cmd == VIDIOC_G_EXT_CTRLS)
				printk("%s: id=%d\n", s, c->id);
			else
				printk("%s: id=%d, value=%d\n", s, c->id, c->value);
		}
		break;
	}
	case VIDIOC_G_CROP:
	case VIDIOC_S_CROP:
	{
		struct v4l2_crop *p=arg;
		/*FIXME: Should also show rect structs */
		printk ("%s: type=%d\n", s, p->type);
		break;
	}
	case VIDIOC_CROPCAP:
#ifdef __OLD_VIDIOC_
	case VIDIOC_CROPCAP_OLD:
#endif
	{
		struct v4l2_cropcap *p=arg;
		/*FIXME: Should also show rect structs */
		printk ("%s: type=%d\n", s, p->type);
		break;
	}
	case VIDIOC_INT_DECODE_VBI_LINE:
	{
		struct v4l2_decode_vbi_line *p=arg;
		printk ("%s: is_second_field=%d, ptr=0x%08lx, line=%d, "
			"type=%d\n", s,
				p->is_second_field,(unsigned long)p->p,p->line,p->type);
		break;
	}
	case VIDIOC_ENUM_FMT:
	{
		struct v4l2_fmtdesc *p=arg;
		printk ("%s: index=%d, type=%d, flags=%d, description=%s,"
			" pixelformat=%d\n", s,
				p->index, p->type, p->flags,p->description,
				p->pixelformat);

		break;
	}
	case VIDIOC_G_FMT:
	case VIDIOC_S_FMT:
	case VIDIOC_TRY_FMT:
	{
		struct v4l2_format *p=arg;
		printk ("%s: type=%s\n", s,
				prt_names(p->type,v4l2_type_names));
		switch (p->type) {
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			v4l_print_pix_fmt (s, &p->fmt.pix);
			break;
#if 0
		/* FIXME: Should be one dump per type*/
		case V4L2_BUF_TYPE_VIDEO_OUTPUT:
		case V4L2_BUF_TYPE_VIDEO_OVERLAY:
		case V4L2_BUF_TYPE_VBI_CAPTURE:
		case V4L2_BUF_TYPE_VBI_OUTPUT:
		case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
		case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		case V4L2_BUF_TYPE_PRIVATE:
			break;
#endif
		default:
			break;
		}
	}
	case VIDIOC_G_FBUF:
	case VIDIOC_S_FBUF:
	{
		struct v4l2_framebuffer *p=arg;
		printk ("%s: capability=%d, flags=%d, base=0x%08lx\n", s,
				p->capability,p->flags, (unsigned long)p->base);
		v4l_print_pix_fmt (s, &p->fmt);
		break;
	}
	case VIDIOC_G_FREQUENCY:
	case VIDIOC_S_FREQUENCY:
	{
		struct v4l2_frequency *p=arg;
		printk ("%s: tuner=%d, type=%d, frequency=%d\n", s,
				p->tuner,p->type,p->frequency);
		break;
	}
	case VIDIOC_ENUMINPUT:
	{
		struct v4l2_input *p=arg;
		printk ("%s: index=%d, name=%s, type=%d, audioset=%d, "
			"tuner=%d, std=%Ld, status=%d\n", s,
				p->index,p->name,p->type,p->audioset,
				p->tuner,
				(unsigned long long)p->std,
				p->status);
		break;
	}
	case VIDIOC_G_JPEGCOMP:
	case VIDIOC_S_JPEGCOMP:
	{
		struct v4l2_jpegcompression *p=arg;
		printk ("%s: quality=%d, APPn=%d, APP_len=%d, COM_len=%d,"
			" jpeg_markers=%d\n", s,
				p->quality,p->APPn,p->APP_len,
				p->COM_len,p->jpeg_markers);
		break;
	}
	case VIDIOC_G_MODULATOR:
	case VIDIOC_S_MODULATOR:
	{
		struct v4l2_modulator *p=arg;
		printk ("%s: index=%d, name=%s, capability=%d, rangelow=%d,"
			" rangehigh=%d, txsubchans=%d\n", s,
				p->index, p->name,p->capability,p->rangelow,
				p->rangehigh,p->txsubchans);
		break;
	}
	case VIDIOC_G_MPEGCOMP:
	case VIDIOC_S_MPEGCOMP:
	{
		struct v4l2_mpeg_compression *p=arg;
		/*FIXME: Several fields not shown */
		printk ("%s: ts_pid_pmt=%d, ts_pid_audio=%d, ts_pid_video=%d, "
			"ts_pid_pcr=%d, ps_size=%d, au_sample_rate=%d, "
			"au_pesid=%c, vi_frame_rate=%d, vi_frames_per_gop=%d, "
			"vi_bframes_count=%d, vi_pesid=%c\n", s,
				p->ts_pid_pmt,p->ts_pid_audio, p->ts_pid_video,
				p->ts_pid_pcr, p->ps_size, p->au_sample_rate,
				p->au_pesid, p->vi_frame_rate,
				p->vi_frames_per_gop, p->vi_bframes_count,
				p->vi_pesid);
		break;
	}
	case VIDIOC_ENUMOUTPUT:
	{
		struct v4l2_output *p=arg;
		printk ("%s: index=%d, name=%s,type=%d, audioset=%d, "
			"modulator=%d, std=%Ld\n",
				s,p->index,p->name,p->type,p->audioset,
				p->modulator,
				(unsigned long long)p->std);
		break;
	}
	case VIDIOC_QUERYCTRL:
	{
		struct v4l2_queryctrl *p=arg;
		printk ("%s: id=%d, type=%d, name=%s, min/max=%d/%d,"
			" step=%d, default=%d, flags=0x%08x\n", s,
				p->id,p->type,p->name,p->minimum,p->maximum,
				p->step,p->default_value,p->flags);
		break;
	}
	case VIDIOC_QUERYMENU:
	{
		struct v4l2_querymenu *p=arg;
		printk ("%s: id=%d, index=%d, name=%s\n", s,
				p->id,p->index,p->name);
		break;
	}
	case VIDIOC_INT_G_REGISTER:
	case VIDIOC_INT_S_REGISTER:
	{
		struct v4l2_register *p=arg;
		printk ("%s: i2c_id=%d, reg=%lu, val=%d\n", s,
				p->i2c_id,p->reg,p->val);

		break;
	}
	case VIDIOC_REQBUFS:
	{
		struct v4l2_requestbuffers *p=arg;
		printk ("%s: count=%d, type=%s, memory=%s\n", s,
				p->count,
				prt_names(p->type,v4l2_type_names),
				prt_names(p->memory,v4l2_memory_names));
		break;
	}
	case VIDIOC_INT_S_AUDIO_ROUTING:
	case VIDIOC_INT_S_VIDEO_ROUTING:
	case VIDIOC_INT_G_AUDIO_ROUTING:
	case VIDIOC_INT_G_VIDEO_ROUTING:
	{
		struct v4l2_routing  *p=arg;
		printk ("%s: input=0x%x, output=0x%x\n", s, p->input, p->output);
		break;
	}
	case VIDIOC_INT_S_CRYSTAL_FREQ:
	{
		struct v4l2_crystal_freq *p=arg;
		printk ("%s: freq=%u, flags=0x%x\n", s, p->freq, p->flags);
		break;
	}
	case VIDIOC_G_SLICED_VBI_CAP:
	{
		struct v4l2_sliced_vbi_cap *p=arg;
		printk ("%s: service_set=%d\n", s,
				p->service_set);
		break;
	}
	case VIDIOC_INT_S_VBI_DATA:
	case VIDIOC_INT_G_VBI_DATA:
	{
		struct v4l2_sliced_vbi_data  *p=arg;
		printk ("%s: id=%d, field=%d, line=%d\n", s,
				p->id, p->field, p->line);
		break;
	}
	case VIDIOC_ENUMSTD:
	{
		struct v4l2_standard *p=arg;
		printk ("%s: index=%d, id=%Ld, name=%s, fps=%d/%d, "
			"framelines=%d\n", s, p->index,
				(unsigned long long)p->id, p->name,
				p->frameperiod.numerator,
				p->frameperiod.denominator,
				p->framelines);

		break;
	}
	case VIDIOC_G_PARM:
	case VIDIOC_S_PARM:
#ifdef __OLD_VIDIOC_
	case VIDIOC_S_PARM_OLD:
#endif
	{
		struct v4l2_streamparm *p=arg;
		printk ("%s: type=%d\n", s, p->type);

		break;
	}
	case VIDIOC_G_TUNER:
	case VIDIOC_S_TUNER:
	{
		struct v4l2_tuner *p=arg;
		printk ("%s: index=%d, name=%s, type=%d, capability=%d, "
			"rangelow=%d, rangehigh=%d, signal=%d, afc=%d, "
			"rxsubchans=%d, audmode=%d\n", s,
				p->index, p->name, p->type,
				p->capability, p->rangelow,p->rangehigh,
				p->rxsubchans, p->audmode, p->signal,
				p->afc);
		break;
	}
#ifdef CONFIG_VIDEO_V4L1_COMPAT
	case VIDIOCGVBIFMT:
	case VIDIOCSVBIFMT:
	{
		struct vbi_format *p=arg;
		printk ("%s: sampling_rate=%d, samples_per_line=%d, "
			"sample_format=%d, start=%d/%d, count=%d/%d, flags=%d\n", s,
				p->sampling_rate,p->samples_per_line,
				p->sample_format,p->start[0],p->start[1],
				p->count[0],p->count[1],p->flags);
		break;
	}
	case VIDIOCGAUDIO:
	case VIDIOCSAUDIO:
	{
		struct video_audio *p=arg;
		printk ("%s: audio=%d, volume=%d, bass=%d, treble=%d, "
			"flags=%d, name=%s, mode=%d, balance=%d, step=%d\n",
				s,p->audio,p->volume,p->bass, p->treble,
				p->flags,p->name,p->mode,p->balance,p->step);
		break;
	}
	case VIDIOCGFBUF:
	case VIDIOCSFBUF:
	{
		struct video_buffer *p=arg;
		printk ("%s: base=%08lx, height=%d, width=%d, depth=%d, "
			"bytesperline=%d\n", s,
				(unsigned long) p->base, p->height, p->width,
				p->depth,p->bytesperline);
		break;
	}
	case VIDIOCGCAP:
	{
		struct video_capability *p=arg;
		printk ("%s: name=%s, type=%d, channels=%d, audios=%d, "
			"maxwidth=%d, maxheight=%d, minwidth=%d, minheight=%d\n",
				s,p->name,p->type,p->channels,p->audios,
				p->maxwidth,p->maxheight,p->minwidth,
				p->minheight);

		break;
	}
	case VIDIOCGCAPTURE:
	case VIDIOCSCAPTURE:
	{
		struct video_capture *p=arg;
		printk ("%s: x=%d, y=%d, width=%d, height=%d, decimation=%d,"
			" flags=%d\n", s,
				p->x, p->y,p->width, p->height,
				p->decimation,p->flags);
		break;
	}
	case VIDIOCGCHAN:
	case VIDIOCSCHAN:
	{
		struct video_channel *p=arg;
		printk ("%s: channel=%d, name=%s, tuners=%d, flags=%d, "
			"type=%d, norm=%d\n", s,
				p->channel,p->name,p->tuners,
				p->flags,p->type,p->norm);

		break;
	}
	case VIDIOCSMICROCODE:
	{
		struct video_code *p=arg;
		printk ("%s: loadwhat=%s, datasize=%d\n", s,
				p->loadwhat,p->datasize);
		break;
	}
	case DECODER_GET_CAPABILITIES:
	{
		struct video_decoder_capability *p=arg;
		printk ("%s: flags=%d, inputs=%d, outputs=%d\n", s,
				p->flags,p->inputs,p->outputs);
		break;
	}
	case DECODER_INIT:
	{
		struct video_decoder_init *p=arg;
		printk ("%s: len=%c\n", s, p->len);
		break;
	}
	case VIDIOCGPLAYINFO:
	{
		struct video_info *p=arg;
		printk ("%s: frame_count=%d, h_size=%d, v_size=%d, "
			"smpte_timecode=%d, picture_type=%d, "
			"temporal_reference=%d, user_data=%s\n", s,
				p->frame_count, p->h_size,
				p->v_size, p->smpte_timecode,
				p->picture_type, p->temporal_reference,
				p->user_data);
		break;
	}
	case VIDIOCKEY:
	{
		struct video_key *p=arg;
		printk ("%s: key=%s, flags=%d\n", s,
				p->key, p->flags);
		break;
	}
	case VIDIOCGMBUF:
	{
		struct video_mbuf *p=arg;
		printk ("%s: size=%d, frames=%d, offsets=0x%08lx\n", s,
				p->size,
				p->frames,
				(unsigned long)p->offsets);
		break;
	}
	case VIDIOCMCAPTURE:
	{
		struct video_mmap *p=arg;
		printk ("%s: frame=%d, height=%d, width=%d, format=%d\n", s,
				p->frame,
				p->height, p->width,
				p->format);
		break;
	}
	case VIDIOCGPICT:
	case VIDIOCSPICT:
	case DECODER_SET_PICTURE:
	{
		struct video_picture *p=arg;

		printk ("%s: brightness=%d, hue=%d, colour=%d, contrast=%d,"
			" whiteness=%d, depth=%d, palette=%d\n", s,
				p->brightness, p->hue, p->colour,
				p->contrast, p->whiteness, p->depth,
				p->palette);
		break;
	}
	case VIDIOCSPLAYMODE:
	{
		struct video_play_mode *p=arg;
		printk ("%s: mode=%d, p1=%d, p2=%d\n", s,
				p->mode,p->p1,p->p2);
		break;
	}
	case VIDIOCGTUNER:
	case VIDIOCSTUNER:
	{
		struct video_tuner *p=arg;
		printk ("%s: tuner=%d, name=%s, rangelow=%ld, rangehigh=%ld, "
			"flags=%d, mode=%d, signal=%d\n", s,
				p->tuner, p->name,p->rangelow, p->rangehigh,
				p->flags,p->mode, p->signal);
		break;
	}
	case VIDIOCGUNIT:
	{
		struct video_unit *p=arg;
		printk ("%s: video=%d, vbi=%d, radio=%d, audio=%d, "
			"teletext=%d\n", s,
				p->video,p->vbi,p->radio,p->audio,p->teletext);
		break;
	}
	case VIDIOCGWIN:
	case VIDIOCSWIN:
	{
		struct video_window *p=arg;
		printk ("%s: x=%d, y=%d, width=%d, height=%d, chromakey=%d,"
			" flags=%d, clipcount=%d\n", s,
				p->x, p->y,p->width, p->height,
				p->chromakey,p->flags,
				p->clipcount);
		break;
	}
	case VIDIOCGFREQ:
	case VIDIOCSFREQ:
	{
		unsigned long *p=arg;
		printk ("%s: value=%lu\n", s, *p);
		break;
	}
#endif
	case VIDIOC_INT_AUDIO_CLOCK_FREQ:
	case VIDIOC_INT_I2S_CLOCK_FREQ:
	case VIDIOC_INT_S_STANDBY:
	{
		u32 *p=arg;

		printk ("%s: value=%d\n", s, *p);
		break;
	}
	case VIDIOC_G_STD:
	case VIDIOC_S_STD:
	case VIDIOC_QUERYSTD:
	{
		v4l2_std_id *p=arg;

		printk ("%s: value=%Lu\n", s, (unsigned long long)*p);
		break;
	}
	}
}

/* ----------------------------------------------------------------- */

/* Helper functions for control handling			     */

/* Check for correctness of the ctrl's value based on the data from
   struct v4l2_queryctrl and the available menu items. Note that
   menu_items may be NULL, in that case it is ignored. */
int v4l2_ctrl_check(struct v4l2_ext_control *ctrl, struct v4l2_queryctrl *qctrl,
		const char **menu_items)
{
	if (qctrl->flags & V4L2_CTRL_FLAG_DISABLED)
		return -EINVAL;
	if (qctrl->flags & V4L2_CTRL_FLAG_GRABBED)
		return -EBUSY;
	if (qctrl->type == V4L2_CTRL_TYPE_BUTTON ||
	    qctrl->type == V4L2_CTRL_TYPE_INTEGER64 ||
	    qctrl->type == V4L2_CTRL_TYPE_CTRL_CLASS)
		return 0;
	if (ctrl->value < qctrl->minimum || ctrl->value > qctrl->maximum)
		return -ERANGE;
	if (qctrl->type == V4L2_CTRL_TYPE_MENU && menu_items != NULL) {
		if (menu_items[ctrl->value] == NULL ||
		    menu_items[ctrl->value][0] == '\0')
			return -EINVAL;
	}
	return 0;
}

/* Returns NULL or a character pointer array containing the menu for
   the given control ID. The pointer array ends with a NULL pointer.
   An empty string signifies a menu entry that is invalid. This allows
   drivers to disable certain options if it is not supported. */
const char **v4l2_ctrl_get_menu(u32 id)
{
	static const char *mpeg_audio_sampling_freq[] = {
		"44.1 kHz",
		"48 kHz",
		"32 kHz",
		NULL
	};
	static const char *mpeg_audio_encoding[] = {
		"Layer I",
		"Layer II",
		"Layer III",
		NULL
	};
	static const char *mpeg_audio_l1_bitrate[] = {
		"32 kbps",
		"64 kbps",
		"96 kbps",
		"128 kbps",
		"160 kbps",
		"192 kbps",
		"224 kbps",
		"256 kbps",
		"288 kbps",
		"320 kbps",
		"352 kbps",
		"384 kbps",
		"416 kbps",
		"448 kbps",
		NULL
	};
	static const char *mpeg_audio_l2_bitrate[] = {
		"32 kbps",
		"48 kbps",
		"56 kbps",
		"64 kbps",
		"80 kbps",
		"96 kbps",
		"112 kbps",
		"128 kbps",
		"160 kbps",
		"192 kbps",
		"224 kbps",
		"256 kbps",
		"320 kbps",
		"384 kbps",
		NULL
	};
	static const char *mpeg_audio_l3_bitrate[] = {
		"32 kbps",
		"40 kbps",
		"48 kbps",
		"56 kbps",
		"64 kbps",
		"80 kbps",
		"96 kbps",
		"112 kbps",
		"128 kbps",
		"160 kbps",
		"192 kbps",
		"224 kbps",
		"256 kbps",
		"320 kbps",
		NULL
	};
	static const char *mpeg_audio_mode[] = {
		"Stereo",
		"Joint Stereo",
		"Dual",
		"Mono",
		NULL
	};
	static const char *mpeg_audio_mode_extension[] = {
		"Bound 4",
		"Bound 8",
		"Bound 12",
		"Bound 16",
		NULL
	};
	static const char *mpeg_audio_emphasis[] = {
		"No Emphasis",
		"50/15 us",
		"CCITT J17",
		NULL
	};
	static const char *mpeg_audio_crc[] = {
		"No CRC",
		"16-bit CRC",
		NULL
	};
	static const char *mpeg_video_encoding[] = {
		"MPEG-1",
		"MPEG-2",
		NULL
	};
	static const char *mpeg_video_aspect[] = {
		"1x1",
		"4x3",
		"16x9",
		"2.21x1",
		NULL
	};
	static const char *mpeg_video_bitrate_mode[] = {
		"Variable Bitrate",
		"Constant Bitrate",
		NULL
	};
	static const char *mpeg_stream_type[] = {
		"MPEG-2 Program Stream",
		"MPEG-2 Transport Stream",
		"MPEG-1 System Stream",
		"MPEG-2 DVD-compatible Stream",
		"MPEG-1 VCD-compatible Stream",
		"MPEG-2 SVCD-compatible Stream",
		NULL
	};
	static const char *mpeg_stream_vbi_fmt[] = {
		"No VBI",
		"Private packet, IVTV format",
		NULL
	};

	switch (id) {
		case V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ:
			return mpeg_audio_sampling_freq;
		case V4L2_CID_MPEG_AUDIO_ENCODING:
			return mpeg_audio_encoding;
		case V4L2_CID_MPEG_AUDIO_L1_BITRATE:
			return mpeg_audio_l1_bitrate;
		case V4L2_CID_MPEG_AUDIO_L2_BITRATE:
			return mpeg_audio_l2_bitrate;
		case V4L2_CID_MPEG_AUDIO_L3_BITRATE:
			return mpeg_audio_l3_bitrate;
		case V4L2_CID_MPEG_AUDIO_MODE:
			return mpeg_audio_mode;
		case V4L2_CID_MPEG_AUDIO_MODE_EXTENSION:
			return mpeg_audio_mode_extension;
		case V4L2_CID_MPEG_AUDIO_EMPHASIS:
			return mpeg_audio_emphasis;
		case V4L2_CID_MPEG_AUDIO_CRC:
			return mpeg_audio_crc;
		case V4L2_CID_MPEG_VIDEO_ENCODING:
			return mpeg_video_encoding;
		case V4L2_CID_MPEG_VIDEO_ASPECT:
			return mpeg_video_aspect;
		case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
			return mpeg_video_bitrate_mode;
		case V4L2_CID_MPEG_STREAM_TYPE:
			return mpeg_stream_type;
		case V4L2_CID_MPEG_STREAM_VBI_FMT:
			return mpeg_stream_vbi_fmt;
		default:
			return NULL;
	}
}

/* Fill in a struct v4l2_queryctrl */
int v4l2_ctrl_query_fill(struct v4l2_queryctrl *qctrl, s32 min, s32 max, s32 step, s32 def)
{
	const char *name;

	qctrl->flags = 0;
	switch (qctrl->id) {
	/* USER controls */
	case V4L2_CID_USER_CLASS: 	name = "User Controls"; break;
	case V4L2_CID_AUDIO_VOLUME: 	name = "Volume"; break;
	case V4L2_CID_AUDIO_MUTE: 	name = "Mute"; break;
	case V4L2_CID_AUDIO_BALANCE: 	name = "Balance"; break;
	case V4L2_CID_AUDIO_BASS: 	name = "Bass"; break;
	case V4L2_CID_AUDIO_TREBLE: 	name = "Treble"; break;
	case V4L2_CID_AUDIO_LOUDNESS: 	name = "Loudness"; break;
	case V4L2_CID_BRIGHTNESS: 	name = "Brightness"; break;
	case V4L2_CID_CONTRAST: 	name = "Contrast"; break;
	case V4L2_CID_SATURATION: 	name = "Saturation"; break;
	case V4L2_CID_HUE: 		name = "Hue"; break;

	/* MPEG controls */
	case V4L2_CID_MPEG_CLASS: 		name = "MPEG Encoder Controls"; break;
	case V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ: name = "Audio Sampling Frequency"; break;
	case V4L2_CID_MPEG_AUDIO_ENCODING: 	name = "Audio Encoding Layer"; break;
	case V4L2_CID_MPEG_AUDIO_L1_BITRATE: 	name = "Audio Layer I Bitrate"; break;
	case V4L2_CID_MPEG_AUDIO_L2_BITRATE: 	name = "Audio Layer II Bitrate"; break;
	case V4L2_CID_MPEG_AUDIO_L3_BITRATE: 	name = "Audio Layer III Bitrate"; break;
	case V4L2_CID_MPEG_AUDIO_MODE: 		name = "Audio Stereo Mode"; break;
	case V4L2_CID_MPEG_AUDIO_MODE_EXTENSION: name = "Audio Stereo Mode Extension"; break;
	case V4L2_CID_MPEG_AUDIO_EMPHASIS: 	name = "Audio Emphasis"; break;
	case V4L2_CID_MPEG_AUDIO_CRC: 		name = "Audio CRC"; break;
	case V4L2_CID_MPEG_VIDEO_ENCODING: 	name = "Video Encoding"; break;
	case V4L2_CID_MPEG_VIDEO_ASPECT: 	name = "Video Aspect"; break;
	case V4L2_CID_MPEG_VIDEO_B_FRAMES: 	name = "Video B Frames"; break;
	case V4L2_CID_MPEG_VIDEO_GOP_SIZE: 	name = "Video GOP Size"; break;
	case V4L2_CID_MPEG_VIDEO_GOP_CLOSURE: 	name = "Video GOP Closure"; break;
	case V4L2_CID_MPEG_VIDEO_PULLDOWN: 	name = "Video Pulldown"; break;
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE: 	name = "Video Bitrate Mode"; break;
	case V4L2_CID_MPEG_VIDEO_BITRATE: 	name = "Video Bitrate"; break;
	case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK: 	name = "Video Peak Bitrate"; break;
	case V4L2_CID_MPEG_VIDEO_TEMPORAL_DECIMATION: name = "Video Temporal Decimation"; break;
	case V4L2_CID_MPEG_STREAM_TYPE: 	name = "Stream Type"; break;
	case V4L2_CID_MPEG_STREAM_PID_PMT: 	name = "Stream PMT Program ID"; break;
	case V4L2_CID_MPEG_STREAM_PID_AUDIO: 	name = "Stream Audio Program ID"; break;
	case V4L2_CID_MPEG_STREAM_PID_VIDEO: 	name = "Stream Video Program ID"; break;
	case V4L2_CID_MPEG_STREAM_PID_PCR: 	name = "Stream PCR Program ID"; break;
	case V4L2_CID_MPEG_STREAM_PES_ID_AUDIO: name = "Stream PES Audio ID"; break;
	case V4L2_CID_MPEG_STREAM_PES_ID_VIDEO: name = "Stream PES Video ID"; break;
	case V4L2_CID_MPEG_STREAM_VBI_FMT:	name = "Stream VBI Format"; break;

	default:
		return -EINVAL;
	}
	switch (qctrl->id) {
	case V4L2_CID_AUDIO_MUTE:
	case V4L2_CID_AUDIO_LOUDNESS:
	case V4L2_CID_MPEG_VIDEO_GOP_CLOSURE:
	case V4L2_CID_MPEG_VIDEO_PULLDOWN:
		qctrl->type = V4L2_CTRL_TYPE_BOOLEAN;
		min = 0;
		max = step = 1;
		break;
	case V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ:
	case V4L2_CID_MPEG_AUDIO_ENCODING:
	case V4L2_CID_MPEG_AUDIO_L1_BITRATE:
	case V4L2_CID_MPEG_AUDIO_L2_BITRATE:
	case V4L2_CID_MPEG_AUDIO_L3_BITRATE:
	case V4L2_CID_MPEG_AUDIO_MODE:
	case V4L2_CID_MPEG_AUDIO_MODE_EXTENSION:
	case V4L2_CID_MPEG_AUDIO_EMPHASIS:
	case V4L2_CID_MPEG_AUDIO_CRC:
	case V4L2_CID_MPEG_VIDEO_ENCODING:
	case V4L2_CID_MPEG_VIDEO_ASPECT:
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
	case V4L2_CID_MPEG_STREAM_TYPE:
	case V4L2_CID_MPEG_STREAM_VBI_FMT:
		qctrl->type = V4L2_CTRL_TYPE_MENU;
		step = 1;
		break;
	case V4L2_CID_USER_CLASS:
	case V4L2_CID_MPEG_CLASS:
		qctrl->type = V4L2_CTRL_TYPE_CTRL_CLASS;
		qctrl->flags |= V4L2_CTRL_FLAG_READ_ONLY;
		min = max = step = def = 0;
		break;
	default:
		qctrl->type = V4L2_CTRL_TYPE_INTEGER;
		break;
	}
	switch (qctrl->id) {
	case V4L2_CID_MPEG_AUDIO_ENCODING:
	case V4L2_CID_MPEG_AUDIO_MODE:
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
	case V4L2_CID_MPEG_VIDEO_B_FRAMES:
	case V4L2_CID_MPEG_STREAM_TYPE:
		qctrl->flags |= V4L2_CTRL_FLAG_UPDATE;
		break;
	case V4L2_CID_AUDIO_VOLUME:
	case V4L2_CID_AUDIO_BALANCE:
	case V4L2_CID_AUDIO_BASS:
	case V4L2_CID_AUDIO_TREBLE:
	case V4L2_CID_BRIGHTNESS:
	case V4L2_CID_CONTRAST:
	case V4L2_CID_SATURATION:
	case V4L2_CID_HUE:
		qctrl->flags |= V4L2_CTRL_FLAG_SLIDER;
		break;
	}
	qctrl->minimum = min;
	qctrl->maximum = max;
	qctrl->step = step;
	qctrl->default_value = def;
	qctrl->reserved[0] = qctrl->reserved[1] = 0;
	snprintf(qctrl->name, sizeof(qctrl->name), name);
	return 0;
}

/* Fill in a struct v4l2_queryctrl with standard values based on
   the control ID. */
int v4l2_ctrl_query_fill_std(struct v4l2_queryctrl *qctrl)
{
	switch (qctrl->id) {
	/* USER controls */
	case V4L2_CID_USER_CLASS:
	case V4L2_CID_MPEG_CLASS:
		return v4l2_ctrl_query_fill(qctrl, 0, 0, 0, 0);
	case V4L2_CID_AUDIO_VOLUME:
		return v4l2_ctrl_query_fill(qctrl, 0, 65535, 65535 / 100, 58880);
	case V4L2_CID_AUDIO_MUTE:
	case V4L2_CID_AUDIO_LOUDNESS:
		return v4l2_ctrl_query_fill(qctrl, 0, 1, 1, 0);
	case V4L2_CID_AUDIO_BALANCE:
	case V4L2_CID_AUDIO_BASS:
	case V4L2_CID_AUDIO_TREBLE:
		return v4l2_ctrl_query_fill(qctrl, 0, 65535, 65535 / 100, 32768);
	case V4L2_CID_BRIGHTNESS:
		return v4l2_ctrl_query_fill(qctrl, 0, 255, 1, 128);
	case V4L2_CID_CONTRAST:
	case V4L2_CID_SATURATION:
		return v4l2_ctrl_query_fill(qctrl, 0, 127, 1, 64);
	case V4L2_CID_HUE:
		return v4l2_ctrl_query_fill(qctrl, -128, 127, 1, 0);

	/* MPEG controls */
	case V4L2_CID_MPEG_AUDIO_SAMPLING_FREQ:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_SAMPLING_FREQ_44100,
				V4L2_MPEG_AUDIO_SAMPLING_FREQ_32000, 1,
				V4L2_MPEG_AUDIO_SAMPLING_FREQ_48000);
	case V4L2_CID_MPEG_AUDIO_ENCODING:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_ENCODING_LAYER_1,
				V4L2_MPEG_AUDIO_ENCODING_LAYER_3, 1,
				V4L2_MPEG_AUDIO_ENCODING_LAYER_2);
	case V4L2_CID_MPEG_AUDIO_L1_BITRATE:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_L1_BITRATE_32K,
				V4L2_MPEG_AUDIO_L1_BITRATE_448K, 1,
				V4L2_MPEG_AUDIO_L1_BITRATE_256K);
	case V4L2_CID_MPEG_AUDIO_L2_BITRATE:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_L2_BITRATE_32K,
				V4L2_MPEG_AUDIO_L2_BITRATE_384K, 1,
				V4L2_MPEG_AUDIO_L2_BITRATE_224K);
	case V4L2_CID_MPEG_AUDIO_L3_BITRATE:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_L3_BITRATE_32K,
				V4L2_MPEG_AUDIO_L3_BITRATE_320K, 1,
				V4L2_MPEG_AUDIO_L3_BITRATE_192K);
	case V4L2_CID_MPEG_AUDIO_MODE:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_MODE_STEREO,
				V4L2_MPEG_AUDIO_MODE_MONO, 1,
				V4L2_MPEG_AUDIO_MODE_STEREO);
	case V4L2_CID_MPEG_AUDIO_MODE_EXTENSION:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_MODE_EXTENSION_BOUND_4,
				V4L2_MPEG_AUDIO_MODE_EXTENSION_BOUND_16, 1,
				V4L2_MPEG_AUDIO_MODE_EXTENSION_BOUND_4);
	case V4L2_CID_MPEG_AUDIO_EMPHASIS:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_EMPHASIS_NONE,
				V4L2_MPEG_AUDIO_EMPHASIS_CCITT_J17, 1,
				V4L2_MPEG_AUDIO_EMPHASIS_NONE);
	case V4L2_CID_MPEG_AUDIO_CRC:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_AUDIO_CRC_NONE,
				V4L2_MPEG_AUDIO_CRC_CRC16, 1,
				V4L2_MPEG_AUDIO_CRC_NONE);
	case V4L2_CID_MPEG_VIDEO_ENCODING:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_VIDEO_ENCODING_MPEG_1,
				V4L2_MPEG_VIDEO_ENCODING_MPEG_2, 1,
				V4L2_MPEG_VIDEO_ENCODING_MPEG_2);
	case V4L2_CID_MPEG_VIDEO_ASPECT:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_VIDEO_ASPECT_1x1,
				V4L2_MPEG_VIDEO_ASPECT_221x100, 1,
				V4L2_MPEG_VIDEO_ASPECT_4x3);
	case V4L2_CID_MPEG_VIDEO_B_FRAMES:
		return v4l2_ctrl_query_fill(qctrl, 0, 33, 1, 2);
	case V4L2_CID_MPEG_VIDEO_GOP_SIZE:
		return v4l2_ctrl_query_fill(qctrl, 1, 34, 1, 12);
	case V4L2_CID_MPEG_VIDEO_GOP_CLOSURE:
		return v4l2_ctrl_query_fill(qctrl, 0, 1, 1, 1);
	case V4L2_CID_MPEG_VIDEO_PULLDOWN:
		return v4l2_ctrl_query_fill(qctrl, 0, 1, 1, 0);
	case V4L2_CID_MPEG_VIDEO_BITRATE_MODE:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_VIDEO_BITRATE_MODE_VBR,
				V4L2_MPEG_VIDEO_BITRATE_MODE_CBR, 1,
				V4L2_MPEG_VIDEO_BITRATE_MODE_VBR);
	case V4L2_CID_MPEG_VIDEO_BITRATE:
		return v4l2_ctrl_query_fill(qctrl, 0, 27000000, 1, 6000000);
	case V4L2_CID_MPEG_VIDEO_BITRATE_PEAK:
		return v4l2_ctrl_query_fill(qctrl, 0, 27000000, 1, 8000000);
	case V4L2_CID_MPEG_VIDEO_TEMPORAL_DECIMATION:
		return v4l2_ctrl_query_fill(qctrl, 0, 255, 1, 0);
	case V4L2_CID_MPEG_STREAM_TYPE:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_STREAM_TYPE_MPEG2_PS,
				V4L2_MPEG_STREAM_TYPE_MPEG2_SVCD, 1,
				V4L2_MPEG_STREAM_TYPE_MPEG2_PS);
	case V4L2_CID_MPEG_STREAM_PID_PMT:
		return v4l2_ctrl_query_fill(qctrl, 0, (1 << 14) - 1, 1, 16);
	case V4L2_CID_MPEG_STREAM_PID_AUDIO:
		return v4l2_ctrl_query_fill(qctrl, 0, (1 << 14) - 1, 1, 260);
	case V4L2_CID_MPEG_STREAM_PID_VIDEO:
		return v4l2_ctrl_query_fill(qctrl, 0, (1 << 14) - 1, 1, 256);
	case V4L2_CID_MPEG_STREAM_PID_PCR:
		return v4l2_ctrl_query_fill(qctrl, 0, (1 << 14) - 1, 1, 259);
	case V4L2_CID_MPEG_STREAM_PES_ID_AUDIO:
		return v4l2_ctrl_query_fill(qctrl, 0, 255, 1, 0);
	case V4L2_CID_MPEG_STREAM_PES_ID_VIDEO:
		return v4l2_ctrl_query_fill(qctrl, 0, 255, 1, 0);
	case V4L2_CID_MPEG_STREAM_VBI_FMT:
		return v4l2_ctrl_query_fill(qctrl,
				V4L2_MPEG_STREAM_VBI_FMT_NONE,
				V4L2_MPEG_STREAM_VBI_FMT_IVTV, 1,
				V4L2_MPEG_STREAM_VBI_FMT_NONE);
	default:
		return -EINVAL;
	}
}

/* Fill in a struct v4l2_querymenu based on the struct v4l2_queryctrl and
   the menu. The qctrl pointer may be NULL, in which case it is ignored. */
int v4l2_ctrl_query_menu(struct v4l2_querymenu *qmenu, struct v4l2_queryctrl *qctrl,
	       const char **menu_items)
{
	int i;

	if (menu_items == NULL ||
	    (qctrl && (qmenu->index < qctrl->minimum || qmenu->index > qctrl->maximum)))
		return -EINVAL;
	for (i = 0; i < qmenu->index && menu_items[i]; i++) ;
	if (menu_items[i] == NULL || menu_items[i][0] == '\0')
		return -EINVAL;
	snprintf(qmenu->name, sizeof(qmenu->name), menu_items[qmenu->index]);
	qmenu->reserved = 0;
	return 0;
}

/* ctrl_classes points to an array of u32 pointers, the last element is
   a NULL pointer. Each u32 array is a 0-terminated array of control IDs.
   Each array must be sorted low to high and belong to the same control
   class. The array of u32 pointer must also be sorted, from low class IDs
   to high class IDs.

   This function returns the first ID that follows after the given ID.
   When no more controls are available 0 is returned. */
u32 v4l2_ctrl_next(const u32 * const * ctrl_classes, u32 id)
{
	u32 ctrl_class;
	const u32 *pctrl;

	/* if no query is desired, then just return the control ID */
	if ((id & V4L2_CTRL_FLAG_NEXT_CTRL) == 0)
		return id;
	if (ctrl_classes == NULL)
		return 0;
	id &= V4L2_CTRL_ID_MASK;
	ctrl_class = V4L2_CTRL_ID2CLASS(id);
	id++;	/* select next control */
	/* find first class that matches (or is greater than) the class of
	   the ID */
	while (*ctrl_classes && V4L2_CTRL_ID2CLASS(**ctrl_classes) < ctrl_class)
		ctrl_classes++;
	/* no more classes */
	if (*ctrl_classes == NULL)
		return 0;
	pctrl = *ctrl_classes;
	/* find first ctrl within the class that is >= ID */
	while (*pctrl && *pctrl < id) pctrl++;
	if (*pctrl)
		return *pctrl;
	/* we are at the end of the controls of the current class. */
	/* continue with next class if available */
	ctrl_classes++;
	if (*ctrl_classes == NULL)
		return 0;
	return **ctrl_classes;
}

/* ----------------------------------------------------------------- */

EXPORT_SYMBOL(v4l2_video_std_construct);

EXPORT_SYMBOL(v4l2_prio_init);
EXPORT_SYMBOL(v4l2_prio_change);
EXPORT_SYMBOL(v4l2_prio_open);
EXPORT_SYMBOL(v4l2_prio_close);
EXPORT_SYMBOL(v4l2_prio_max);
EXPORT_SYMBOL(v4l2_prio_check);

EXPORT_SYMBOL(v4l2_field_names);
EXPORT_SYMBOL(v4l2_type_names);
EXPORT_SYMBOL(v4l_printk_ioctl);
EXPORT_SYMBOL(v4l_printk_ioctl_arg);

EXPORT_SYMBOL(v4l2_ctrl_next);
EXPORT_SYMBOL(v4l2_ctrl_check);
EXPORT_SYMBOL(v4l2_ctrl_get_menu);
EXPORT_SYMBOL(v4l2_ctrl_query_menu);
EXPORT_SYMBOL(v4l2_ctrl_query_fill);
EXPORT_SYMBOL(v4l2_ctrl_query_fill_std);

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
