/*
 * $Id: compat.h,v 1.44 2006/01/15 09:35:16 mchehab Exp $
 */

#ifndef _COMPAT_H
#define _COMPAT_H

#include <linux/i2c-id.h>
#include <linux/pm.h>
#include <linux/version.h>
#include <linux/utsname.h>
#include <linux/sched.h>
#include "config-compat.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,18)
# define minor(x) MINOR(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include <linux/delay.h>
# define need_resched() (current->need_resched)

#define work_struct tq_struct
#else
#include <linux/device.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,19)
# define BUG_ON(condition) do { if ((condition)!=0) BUG(); } while(0)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,23)
# define irqreturn_t void
# define IRQ_RETVAL(foobar)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
# define strlcpy(dest,src,len) strncpy(dest,src,(len)-1)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
# define iminor(inode) minor(inode->i_rdev)
#endif

#if defined(I2C_ADAP_CLASS_TV_ANALOG) && !defined(I2C_CLASS_TV_ANALOG)
# define  I2C_CLASS_TV_ANALOG  I2C_ADAP_CLASS_TV_ANALOG
# define  I2C_CLASS_TV_DIGITAL I2C_ADAP_CLASS_TV_DIGITAL
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
# define __user
# define __kernel
# define __iomem
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
# define pm_message_t                      u32
# define pci_choose_state(pci_dev, state)  (state)
# define PCI_D0                            (0)
# define assert_spin_locked(foobar)
#endif
#if !defined(I2C_ALGO_SAA7134)
#define I2C_ALGO_SAA7134 I2C_HW_B_BT848
#endif
#if !defined(I2C_HW_B_CX2388x)
# define I2C_HW_B_CX2388x I2C_HW_B_BT848
#endif
#if !defined(I2C_HW_SAA7134)
# define I2C_HW_SAA7134 I2C_ALGO_SAA7134
#endif
#if !defined(I2C_HW_SAA7146)
# define I2C_HW_SAA7146 I2C_ALGO_SAA7146
#endif

#if !defined(I2C_HW_B_EM2820)
#define I2C_HW_B_EM2820 0x99
#endif

#ifndef I2C_M_IGNORE_NAK
# define I2C_M_IGNORE_NAK 0x1000
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
#define __le32 __u32
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7))
static inline unsigned long msecs_to_jiffies(const unsigned int m)
{
#if HZ <= 1000 && !(1000 % HZ)
	return (m + (1000 / HZ) - 1) / (1000 / HZ);
#else
#if HZ > 1000 && !(HZ % 1000)
	return m * (HZ / 1000);
#else
	return (m * HZ + 999) / 1000;
#endif
#endif
}
static inline unsigned int jiffies_to_msecs(const unsigned long j)
{
#if HZ <= 1000 && !(1000 % HZ)
	return (1000 / HZ) * j;
#else
#if HZ > 1000 && !(HZ % 1000)
	return (j + (HZ / 1000) - 1)/(HZ / 1000);
#else
	return (j * 1000) / HZ;
#endif
#endif
}
static inline void msleep(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs);
	while (timeout) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
static inline unsigned long msleep_interruptible(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs);

	while (timeout) {
		set_current_state(TASK_INTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
	return jiffies_to_msecs(timeout);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
/* some keys from 2.6.x which are not (yet?) in 2.4.x */
# define KEY_PLAY                207
# define KEY_PRINT		 210
# define KEY_EMAIL         215
# define KEY_SEARCH              217
# define KEY_SELECT 		 0x161
# define KEY_GOTO                0x162
# define KEY_INFO                0x166
# define KEY_CHANNEL             0x16b
# define KEY_LANGUAGE            0x170
# define KEY_SUBTITLE		 0x172
# define KEY_ZOOM                0x174
# define KEY_MODE		 0x175
# define KEY_TV                  0x179
# define KEY_CD                  0x17f
# define KEY_TUNER               0x182
# define KEY_TEXT                0x184
# define KEY_DVD		 0x185
# define KEY_AUDIO               0x188
# define KEY_VIDEO               0x189
# define KEY_RED                 0x18e
# define KEY_GREEN               0x18f
# define KEY_YELLOW              0x190
# define KEY_BLUE                0x191
# define KEY_CHANNELUP           0x192
# define KEY_CHANNELDOWN         0x193
# define KEY_RESTART		 0x198
# define KEY_SHUFFLE     	 0x19a
# define KEY_NEXT                0x197
# define KEY_RADIO               0x181
# define KEY_PREVIOUS            0x19c
# define KEY_MHP                 0x16f
# define KEY_EPG                 0x16d
# define KEY_FASTFORWARD         208
# define KEY_LIST                0x18b
# define KEY_LAST                0x195
# define KEY_CLEAR               0x163
# define KEY_AUX                 0x186
# define KEY_SCREEN              0x177
# define KEY_PC                  0x178
# define KEY_MEDIA               226
# define KEY_SLOW                0x199
# define KEY_OK                  0x160
# define KEY_DIGITS              0x19d
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
# define KEY_SEND		231
# define KEY_REPLY		232
# define KEY_FORWARDMAIL	233
# define KEY_SAVE		234
# define KEY_DOCUMENTS		235
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#define container_of(ptr, type, member) ({                      \
	const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
	(type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
#include <linux/mm.h>
static inline unsigned long vmalloc_to_pfn(void * vmalloc_addr)
{
    return page_to_pfn(vmalloc_to_page(vmalloc_addr));
}

#ifndef wait_event_timeout
#define wait_event_timeout(wq, condition, timeout)                   	     \
({                                                                           \
     long __ret = timeout;                                                   \
     if (!(condition))                                                       \
     do {                                                                    \
	     DEFINE_WAIT(__wait);                                            \
	     for (;;) {                                                      \
		     prepare_to_wait(&wq, &__wait, TASK_UNINTERRUPTIBLE);    \
		     if (condition)                                          \
			 break;                                              \
		     __ret = schedule_timeout(__ret);                        \
		     if (!__ret)                                             \
			 break;                                              \
	     }                                                               \
	     finish_wait(&wq, &__wait);                                      \
     } while (0);							     \
     __ret;                                                                  \
})
#endif

#define remap_pfn_range remap_page_range

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
#ifndef kcalloc
#define kcalloc(n,size,flags)			\
({						\
  void * __ret = NULL;				\
  __ret = kmalloc(n * size, flags);		\
  if (__ret)					\
	 memset(__ret, 0, n * size);		\
  __ret;					\
})
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
#ifndef kzalloc
#define kzalloc(size, flags)				\
({							\
	void *__ret = kmalloc(size, flags);		\
	if (__ret)					\
		memset(__ret, 0, size);			\
	__ret;						\
})
#endif
#endif

/* The class_device system didn't appear until 2.5.69 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#define class_device_create_file(a, b) (0)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
# define class_device_create(a, b, c, d, e, f, g, h) class_simple_device_add(a, c, d, e, f, g, h)
# define class_device_destroy(a, b) class_simple_device_remove(b)
# define class_create(a, b) class_simple_create(a, b)
# define class_destroy(a) class_simple_destroy(a)
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
# define class_device_create(a, b, c, d, e, f, g, h) class_device_create(a, c, d, e, f, g, h)
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
# define input_allocate_device() kzalloc(sizeof(struct input_dev),GFP_KERNEL);
# define input_free_device(input_dev) kfree(input_dev)
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,15)
#define DEFINE_MUTEX(a) DECLARE_MUTEX(a)
#define mutex_lock_interruptible(a) down_interruptible(a)
#define mutex_unlock(a) up(a)
#define mutex_lock(a) down(a)
#define mutex_init(a) init_MUTEX(a)
#define mutex_trylock(a) down_trylock(a)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
static inline signed long __sched
schedule_timeout_interruptible(signed long timeout)
{
	__set_current_state(TASK_INTERRUPTIBLE);
	return schedule_timeout(timeout);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#define IRQF_SHARED		SA_SHIRQ
#define IRQF_DISABLED		SA_INTERRUPT
#endif

#endif
/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
