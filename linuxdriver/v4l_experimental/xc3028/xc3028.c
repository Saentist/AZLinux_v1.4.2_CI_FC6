/*

   Xceive -  xc3028 tuner interface

   Copyright (c) 2006 Markus Rechberger <mrechberger@gmail.com>


TODO: - remove em28xx dependency
      - add channel locking (requires some more reverse engineering)
      - try to get the datasheet from Xceive :)

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

#include <linux/i2c.h>
#include <linux/usb.h>
#include "compat.h"
#include <linux/videodev.h>
#include "em28xx.h"
#include <linux/firmware.h>
#include <linux/delay.h>
#include <media/tuner.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include "i2c-compat.h"
#endif

#define XC3028_DEFAULT_FIRMWARE "xceive_xc_3028.fw"

int xceive_set_color(struct i2c_client *c);

/* ---------------------------------------------------------------------- */

int xc3028_probe(struct i2c_client *c)
{
	printk("xc3028: probe function unknown\n");
	return -1;
}

static void xc3028_set_tv_freq(struct i2c_client *c, unsigned int freq){
	// the frequency is just shifted and there's a 1:1 relation for all frequencies
	// E11 is Das Erste in Germany/Ulm all other channels match their frequency too

	unsigned char chanbuf[4];
	freq<<=2;
	chanbuf[0]=0;
	chanbuf[1]=0;
	chanbuf[2]=(freq&0xff00)>>8;
	chanbuf[3]=freq&0x00ff;
	i2c_master_send(c,"\xa0\x00\x00\x00",4);
	i2c_master_send(c,"\x1e\x1f\x13\x87\x18\x02\x93\x91\x44\x86\x96\x8c",12);
	i2c_master_send(c,"\x00\x8c",2);
	i2c_master_send(c,"\x80\x02\x00\x00",4);
	i2c_master_send(c,chanbuf,4);
}

int xc3028_init(struct i2c_client *c)
{
	struct tuner *t = i2c_get_clientdata(c);
	int ret;
	const struct firmware *fw = NULL;
	u8 *firmware;
	char *fwoffset;
	u8 initoffset[8];
	u8 linebuffer[100];
	int splitrange;
	struct em28xx *dev;
	int i=0;
	int d=0;
	int txtlen;
	long fwoff;
	size_t firmware_size = 5000;

	// request firmware from /lib/firmware, note that the file got extracted by the convert application I wrote and which is available
	// on linuxtv.org / xc3028

	ret = request_firmware(&fw, XC3028_DEFAULT_FIRMWARE, &t->i2c.dev);
	if (ret) {
		printk("xc3028: no firmware uploaded please check %s\n",XC3028_DEFAULT_FIRMWARE);
		return ret;
	}
	firmware = fw->data;
	firmware_size = fw->size;

	// small firmware check, both firmwares I have are between 6 and 7k bytes

	if(fw->size>7000||fw->size<6000){
		printk("xc3028: wrong firmware provided!\n");
		release_firmware(fw);
		return(ret);
	}
	for(i=0;i<8&&firmware[i]!='\n';i++);
	txtlen=i;
	firmware[i++]=0;
	fwoff=simple_strtol(firmware,&fwoffset,10);
	if(fwoff>fw->size){
		printk("xc3028: firmware offset doesn't match!\n");
		release_firmware(fw);
		return(-1);
	}

	linebuffer[d++]=0x2a;
	dev=c->adapter->algo_data;

	// 0x08 is a GPIO address of the em28xx has to get replaced with something generic here

	dev->em28xx_write_regs(dev, 0x08, "\x6d", 1);
	mdelay(100);
	dev->em28xx_write_regs(dev, 0x08, "\x7d", 1);
	mdelay(100);

	// the firmware always starts with 0x2a + 0x40 bytes payload I use to add the offset of the first part
	// as the first line into the firmware binary

	while(i!=fw->size){
		linebuffer[d++]=firmware[i];
		if((d%64==0&&d!=0)||i==fwoff+txtlen){
			i2c_master_send(c,linebuffer,d);
			if(i==(fwoff+txtlen)){
				i2c_master_send(c,"\x02\x02",2);
				i2c_master_send(c,"\x02\x03",2);
				i2c_master_send(c,"\x00\x8c",2);
				i2c_master_send(c,"\x00\x00\x00\x00",4);
				// another reset here
				dev->em28xx_write_regs(dev, 0x08, "\x6d", 1);
				mdelay(100);
				dev->em28xx_write_regs(dev, 0x08, "\x7d", 1);
				mdelay(100);

			}
			linebuffer[0]=0x2a;
			d=1;
		}
		i++;
	}
	printk("xc3024: Firmware uploaded\n");

	firmware[firmware_size-1]=0;

	release_firmware(fw);

	/* MAGIC VALUES */
	i2c_master_send(c,"\x13\x39",2);
	i2c_master_send(c,"\x0c\x80\xf0\xf7\x3e\x75\xc1\x8a\xe4\x02\x00",11);
	i2c_master_send(c,"\x05\x0f\xee\xaa\x5f\xea\x90",7);
	i2c_master_send(c,"\x06\x00\x0a\x4d\x8c\xf2\xd8\xcf\x30\x79\x9f",11);
	i2c_master_send(c,"\x0b\x0d\xa4\x6c",4);
	i2c_master_send(c,"\x0a\x01\x67\x24\x40\x08\xc3\x20\x10\x64\x3c\xfa\xf7\xe1\x0c\x2c",0x10);
	i2c_master_send(c,"\x09\x0b",0x2);
	i2c_master_send(c,"\x10\x13",0x2);
	i2c_master_send(c,"\x16\x12",0x2);
	i2c_master_send(c,"\x1f\x02",0x2);
	i2c_master_send(c,"\x21\x02",0x2);
	i2c_master_send(c,"\x01\x02",0x2);
	i2c_master_send(c,"\x2b\x10",0x2);
	i2c_master_send(c,"\x02\x02",0x2);
	i2c_master_send(c,"\x02\x03",0x2);
	i2c_master_send(c,"\x00\x8c",0x2);

#if 0
	// if set video will mostly be black/white  - if set_color is called instead the video will have color
	i2c_master_send(c,"\x80\x01\x00\x00",0x4);
	i2c_master_send(c,"\x00\x5e\x00\x29",0x4);
	i2c_master_send(c,"\x2b\x1a",0x2);
#endif
	xceive_set_color(c);
	t->set_tv_freq    = xc3028_set_tv_freq;
	return(0);
}

int xceive_set_color(struct i2c_client *c){
	// I found this codeblock within the sniffed logfile it got called as it is a several times after 0x00 0x04 tuner settings are made
	i2c_master_send(c,"\x80\x01\x00\x00",4);
	i2c_master_send(c,"\x00\x5e\x00\x29",4);
	i2c_master_send(c,"\x2b\x1a",2);
	i2c_master_send(c,"\x2b\x1b",2);
	i2c_master_send(c,"\x14\x01\x6c\x25\x82\x38\xa4\x49\xa9\x24\x96\x69",0x0c);
	i2c_master_send(c,"\x13\x14\x08\x30\x10\x6c\x18\x12\x0d\x19\x32\xad",0x0c);
	i2c_master_send(c,"\x0d\x01\x4b\x03\x97\x55\xc7\xd7\x00\xa1\xeb\x8f\x5c",0x0d);
	i2c_master_send(c,"\x1a\x00\x00\x16\x8a\x40\x00\x00\x00\x20",0x0a);
	i2c_master_send(c,"\x2d\x01",2);
	i2c_master_send(c,"\x18\x01",2);
	i2c_master_send(c,"\x1b\x01\xb6\x15\x16\xb1\xa6\xd2\xa9\x12\x41\x66",0x0c);
	i2c_master_send(c,"\x1d\x00",2);
	i2c_master_send(c,"\x0f\x00\x29\x56\xb0\x00\xb6",0x07);
	i2c_master_send(c,"\x20\x00",0x02);
	i2c_master_send(c,"\x1e\x10\x32\x00\x00\x02\xe4\x81\x00\x06\xa9\x04",0x0c);
	i2c_master_send(c,"\x22\x29",0x02);
	i2c_master_send(c,"\x23\x06",0x02);
	i2c_master_send(c,"\x25\x00\x09\x90\x09\x06\x64\x02\x41",0x09);
	i2c_master_send(c,"\x26\xcc",0x02);
	i2c_master_send(c,"\x29\x40",0x02);
	i2c_master_send(c,"\x21\x03",0x02);
	i2c_master_send(c,"\x00\x8c",0x02);
	i2c_master_send(c,"\x00\x00\x00\x00",0x04);
	i2c_master_send(c,"\x00\x04",0x02);
	return(0);
}


/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
