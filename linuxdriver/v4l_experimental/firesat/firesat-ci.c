#include "firesat-ci.h"
#include "firesat.h"
#include "avc_api.h"

#include <linux/dvb/ca.h>

#include "dvbdev.h"

static int firesat_ca_do_ioctl(struct inode *inode, struct file *file, unsigned int cmd, void *parg) {
	struct firesat *firesat = (struct firesat*)((struct dvb_device*)file->private_data)->priv;
	int err;

//	printk(KERN_INFO "%s: ioctl %d\n",__FUNCTION__,cmd);

	switch(cmd) {
	case CA_RESET:
		err = AVCResetTPDU(firesat);
//		if(err==0)
			mdelay(3000);
		break;
	case CA_GET_CAP: {
		ca_caps_t *cap=(ca_caps_t*)parg;
		cap->slot_num = 1;
		cap->slot_type = CA_CI_LINK;
		cap->descr_num = 1;
		cap->descr_type = CA_DSS; // ### peter fragen!

		err = 0;
		break;
	}
	case CA_GET_SLOT_INFO: {
		ca_slot_info_t *slot=(ca_slot_info_t*)parg;
		if(slot->num == 0) {
			slot->type = CA_CI | CA_CI_LINK | CA_DESCR;
			slot->flags = CA_CI_MODULE_PRESENT | CA_CI_MODULE_READY;
		} else {
			slot->type = 0;
			slot->flags = 0;
		}
		err = 0;
		break;
	}
	default:
			err=-EINVAL;
	}
	return err;
}

static int firesat_ca_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) {
	return dvb_usercopy(inode, file, cmd, arg, firesat_ca_do_ioctl);
}

static ssize_t firesat_ca_io_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
	struct firesat *firesat = (struct firesat*)((struct dvb_device*)file->private_data)->priv;
	char *data=kmalloc(count,GFP_KERNEL);
//	printk(KERN_INFO "%s: count = %d\n",__FUNCTION__, count);
	copy_from_user(data,buf,count);
	int r=AVCWriteTPDU(firesat, data, count);
	kfree(data);
	if(!r)
		return count;
	printk("%s: failed writing data\n",__FUNCTION__);
	return 0;
}

static ssize_t firesat_ca_io_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
	struct firesat *firesat = (struct firesat*)((struct dvb_device*)file->private_data)->priv;
	char *data=kmalloc(count,GFP_KERNEL);
//	printk(KERN_INFO "%s: count = %d\n",__FUNCTION__,count);
	int r=AVCReadTPDU(firesat, data, &count);
	if(!r) {
		copy_to_user(buf,data,count);
		kfree(data);
		return count;
	}
	kfree(data);
	return 0;
}

static int firesat_ca_io_open(struct inode *inode, struct file *file) {
	printk(KERN_INFO "%s!\n",__FUNCTION__);
	return dvb_generic_open(inode, file);
}

static int firesat_ca_io_release(struct inode *inode, struct file *file) {
	printk(KERN_INFO "%s!\n",__FUNCTION__);
	return dvb_generic_release(inode, file);
}

static unsigned int firesat_ca_io_poll(struct file *file, poll_table *wait) {
//	printk(KERN_INFO "%s!\n",__FUNCTION__);
	return POLLIN;
}

static struct file_operations firesat_ca_fops = {
	.owner = THIS_MODULE,
	.read = firesat_ca_io_read,
	.write = firesat_ca_io_write,
	.ioctl = firesat_ca_ioctl,
	.open = firesat_ca_io_open,
	.release = firesat_ca_io_release,
	.poll = firesat_ca_io_poll,
};

static struct dvb_device firesat_ca = {
	.priv = NULL,
	.users = 1,
	.readers = 1,
	.writers = 1,
	.fops = &firesat_ca_fops,
};

int firesat_ca_init(struct firesat *firesat) {
	int ret = dvb_register_device(firesat->adapter, &firesat->cadev, &firesat_ca, firesat, DVB_DEVICE_CA);
	if(ret) return ret;
	AVCResetTPDU(firesat);
	// avoid unnecessary delays, we're not talking to the CI yet anyways
	return 0;
}

void firesat_ca_release(struct firesat *firesat) {
	dvb_unregister_device(firesat->cadev);
}

