/*
 * Guillemot Maxi Radio FM 2000 PCI radio card driver for Linux
 * (C) 2001 Dimitromanolakis Apostolos <apdim@grecian.net>
 *
 * Based in the radio Maestro PCI driver. Actually it uses the same chip
 * for radio but different pci controller.
 *
 * I didn't have any specs I reversed engineered the protocol from
 * the windows driver (radio.dll).
 *
 * The card uses the TEA5757 chip that includes a search function but it
 * is useless as I haven't found any way to read back the frequency. If
 * anybody does please mail me.
 *
 * For the pdf file see:
 * http://www.semiconductors.philips.com/pip/TEA5757H/V1
 *
 *
 * CHANGES:
 *   0.75b
 *     - better pci interface thanks to Francois Romieu <romieu@cogenit.fr>
 *
 *   0.75      Sun Feb  4 22:51:27 EET 2001
 *     - tiding up
 *     - removed support for multiple devices as it didn't work anyway
 *
 * BUGS:
 *   - card unmutes if you change frequency
 *
 * Converted to V4L2 API by Mauro Carvalho Chehab <mchehab@infradead.org>
 */


#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "compat.h"
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,15)
#include <linux/mutex.h>
#endif

#include <linux/pci.h>
#include <linux/videodev2.h>
#include <media/v4l2-common.h>

#define DRIVER_VERSION	"0.76"

#include <linux/version.h>      /* for KERNEL_VERSION MACRO     */
#define RADIO_VERSION KERNEL_VERSION(0,7,6)

static struct v4l2_queryctrl radio_qctrl[] = {
	{
		.id            = V4L2_CID_AUDIO_MUTE,
		.name          = "Mute",
		.minimum       = 0,
		.maximum       = 1,
		.default_value = 1,
		.type          = V4L2_CTRL_TYPE_BOOLEAN,
	}
};

#ifndef PCI_VENDOR_ID_GUILLEMOT
#define PCI_VENDOR_ID_GUILLEMOT 0x5046
#endif

#ifndef PCI_DEVICE_ID_GUILLEMOT
#define PCI_DEVICE_ID_GUILLEMOT_MAXIRADIO 0x1001
#endif


/* TEA5757 pin mappings */
static const int clk = 1, data = 2, wren = 4, mo_st = 8, power = 16 ;

static int radio_nr = -1;
module_param(radio_nr, int, 0);


#define FREQ_LO		 50*16000
#define FREQ_HI		150*16000

#define FREQ_IF         171200 /* 10.7*16000   */
#define FREQ_STEP       200    /* 12.5*16      */

#define FREQ2BITS(x)	((( (unsigned int)(x)+FREQ_IF+(FREQ_STEP<<1))\
			/(FREQ_STEP<<2))<<2) /* (x==fmhz*16*1000) -> bits */

#define BITS2FREQ(x)	((x) * FREQ_STEP - FREQ_IF)


static int radio_ioctl(struct inode *inode, struct file *file,
		       unsigned int cmd, unsigned long arg);

static struct file_operations maxiradio_fops = {
	.owner		= THIS_MODULE,
	.open           = video_exclusive_open,
	.release        = video_exclusive_release,
	.ioctl		= radio_ioctl,
	.compat_ioctl	= v4l_compat_ioctl32,
	.llseek         = no_llseek,
};
static struct video_device maxiradio_radio =
{
	.owner		= THIS_MODULE,
	.name		= "Maxi Radio FM2000 radio",
	.type		= VID_TYPE_TUNER,
	.fops           = &maxiradio_fops,
};

static struct radio_device
{
	__u16	io,	/* base of radio io */
		muted,	/* VIDEO_AUDIO_MUTE */
		stereo,	/* VIDEO_TUNER_STEREO_ON */
		tuned;	/* signal strength (0 or 0xffff) */

	unsigned long freq;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,15)
	struct mutex lock;
#else
	struct semaphore lock;
#endif
} radio_unit = {0, 0, 0, 0, };


static void outbit(unsigned long bit, __u16 io)
{
	if(bit != 0)
		{
			outb(  power|wren|data     ,io); udelay(4);
			outb(  power|wren|data|clk ,io); udelay(4);
			outb(  power|wren|data     ,io); udelay(4);
		}
	else
		{
			outb(  power|wren          ,io); udelay(4);
			outb(  power|wren|clk      ,io); udelay(4);
			outb(  power|wren          ,io); udelay(4);
		}
}

static void turn_power(__u16 io, int p)
{
	if(p != 0) outb(power, io); else outb(0,io);
}


static void set_freq(__u16 io, __u32 data)
{
	unsigned long int si;
	int bl;

	/* TEA5757 shift register bits (see pdf) */

	outbit(0,io); // 24  search
	outbit(1,io); // 23  search up/down

	outbit(0,io); // 22  stereo/mono

	outbit(0,io); // 21  band
	outbit(0,io); // 20  band (only 00=FM works I think)

	outbit(0,io); // 19  port ?
	outbit(0,io); // 18  port ?

	outbit(0,io); // 17  search level
	outbit(0,io); // 16  search level

	si = 0x8000;
	for(bl = 1; bl <= 16 ; bl++) { outbit(data & si,io); si >>=1; }

	outb(power,io);
}

static int get_stereo(__u16 io)
{
	outb(power,io); udelay(4);
	return !(inb(io) & mo_st);
}

static int get_tune(__u16 io)
{
	outb(power+clk,io); udelay(4);
	return !(inb(io) & mo_st);
}


static inline int radio_function(struct inode *inode, struct file *file,
				 unsigned int cmd, void *arg)
{
	struct video_device *dev = video_devdata(file);
	struct radio_device *card=dev->priv;

	switch(cmd) {
		case VIDIOC_QUERYCAP:
		{
			struct v4l2_capability *v = arg;
			memset(v,0,sizeof(*v));
			strlcpy(v->driver, "radio-maxiradio", sizeof (v->driver));
			strlcpy(v->card, "Maxi Radio FM2000 radio", sizeof (v->card));
			sprintf(v->bus_info,"ISA");
			v->version = RADIO_VERSION;
			v->capabilities = V4L2_CAP_TUNER;

			return 0;
		}
		case VIDIOC_G_TUNER:
		{
			struct v4l2_tuner *v = arg;

			if (v->index > 0)
				return -EINVAL;

			memset(v,0,sizeof(*v));
			strcpy(v->name, "FM");
			v->type = V4L2_TUNER_RADIO;

			v->rangelow=FREQ_LO;
			v->rangehigh=FREQ_HI;
			v->rxsubchans =V4L2_TUNER_SUB_MONO|V4L2_TUNER_SUB_STEREO;
			v->capability=V4L2_TUNER_CAP_LOW;
			if(get_stereo(card->io))
				v->audmode = V4L2_TUNER_MODE_STEREO;
			else
				v->audmode = V4L2_TUNER_MODE_MONO;
			v->signal=0xffff*get_tune(card->io);

			return 0;
		}
		case VIDIOC_S_TUNER:
		{
			struct v4l2_tuner *v = arg;

			if (v->index > 0)
				return -EINVAL;

			return 0;
		}
		case VIDIOC_S_FREQUENCY:
		{
			struct v4l2_frequency *f = arg;

			if (f->frequency < FREQ_LO || f->frequency > FREQ_HI)
				return -EINVAL;

			card->freq = f->frequency;
			set_freq(card->io, FREQ2BITS(card->freq));
			msleep(125);
			return 0;
		}
		case VIDIOC_G_FREQUENCY:
		{
			struct v4l2_frequency *f = arg;

			f->type = V4L2_TUNER_RADIO;
			f->frequency = card->freq;

			return 0;
		}
		case VIDIOC_QUERYCTRL:
		{
			struct v4l2_queryctrl *qc = arg;
			int i;

			for (i = 0; i < ARRAY_SIZE(radio_qctrl); i++) {
				if (qc->id && qc->id == radio_qctrl[i].id) {
					memcpy(qc, &(radio_qctrl[i]),
								sizeof(*qc));
					return (0);
				}
			}
			return -EINVAL;
		}
		case VIDIOC_G_CTRL:
		{
			struct v4l2_control *ctrl= arg;

			switch (ctrl->id) {
				case V4L2_CID_AUDIO_MUTE:
					ctrl->value=card->muted;
					return (0);
			}
			return -EINVAL;
		}
		case VIDIOC_S_CTRL:
		{
			struct v4l2_control *ctrl= arg;

			switch (ctrl->id) {
				case V4L2_CID_AUDIO_MUTE:
					card->muted = ctrl->value;
					if(card->muted)
						turn_power(card->io, 0);
					else
						set_freq(card->io, FREQ2BITS(card->freq));
					return 0;
			}
			return -EINVAL;
		}

		default:
			return v4l_compat_translate_ioctl(inode,file,cmd,arg,
							  radio_function);

#if 0 /* Probably, this is useless */
		case VIDIOCGUNIT: {
			struct video_unit *v = arg;

			v->video=VIDEO_NO_UNIT;
			v->vbi=VIDEO_NO_UNIT;
			v->radio=dev->minor;
			v->audio=0;
			v->teletext=VIDEO_NO_UNIT;
			return 0;
		}
#endif
	}
}

static int radio_ioctl(struct inode *inode, struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	struct video_device *dev = video_devdata(file);
	struct radio_device *card=dev->priv;
	int ret;

	mutex_lock(&card->lock);
	ret = video_usercopy(inode, file, cmd, arg, radio_function);
	mutex_unlock(&card->lock);
	return ret;
}

MODULE_AUTHOR("Dimitromanolakis Apostolos, apdim@grecian.net");
MODULE_DESCRIPTION("Radio driver for the Guillemot Maxi Radio FM2000 radio.");
MODULE_LICENSE("GPL");


static int __devinit maxiradio_init_one(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	if(!request_region(pci_resource_start(pdev, 0),
			   pci_resource_len(pdev, 0), "Maxi Radio FM 2000")) {
		printk(KERN_ERR "radio-maxiradio: can't reserve I/O ports\n");
		goto err_out;
	}

	if (pci_enable_device(pdev))
		goto err_out_free_region;

	radio_unit.io = pci_resource_start(pdev, 0);
	mutex_init(&radio_unit.lock);
	maxiradio_radio.priv = &radio_unit;

	if(video_register_device(&maxiradio_radio, VFL_TYPE_RADIO, radio_nr)==-1) {
		printk("radio-maxiradio: can't register device!");
		goto err_out_free_region;
	}

	printk(KERN_INFO "radio-maxiradio: version "
	       DRIVER_VERSION
	       " time "
	       __TIME__ "  "
	       __DATE__
	       "\n");

	printk(KERN_INFO "radio-maxiradio: found Guillemot MAXI Radio device (io = 0x%x)\n",
	       radio_unit.io);
	return 0;

err_out_free_region:
	release_region(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0));
err_out:
	return -ENODEV;
}

static void __devexit maxiradio_remove_one(struct pci_dev *pdev)
{
	video_unregister_device(&maxiradio_radio);
	release_region(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0));
}

static struct pci_device_id maxiradio_pci_tbl[] = {
	{ PCI_VENDOR_ID_GUILLEMOT, PCI_DEVICE_ID_GUILLEMOT_MAXIRADIO,
		PCI_ANY_ID, PCI_ANY_ID, },
	{ 0,}
};

MODULE_DEVICE_TABLE(pci, maxiradio_pci_tbl);

static struct pci_driver maxiradio_driver = {
	.name		= "radio-maxiradio",
	.id_table	= maxiradio_pci_tbl,
	.probe		= maxiradio_init_one,
	.remove		= __devexit_p(maxiradio_remove_one),
};

static int __init maxiradio_radio_init(void)
{
	return pci_register_driver(&maxiradio_driver);
}

static void __exit maxiradio_radio_exit(void)
{
	pci_unregister_driver(&maxiradio_driver);
}

module_init(maxiradio_radio_init);
module_exit(maxiradio_radio_exit);
