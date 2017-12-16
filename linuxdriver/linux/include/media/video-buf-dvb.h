#include <dvbdev.h>
#include <dmxdev.h>
#include <dvb_demux.h>
#include <dvb_net.h>
#include <dvb_frontend.h>

struct videobuf_dvb {
	/* filling that the job of the driver */
	char                       *name;
	struct dvb_frontend        *frontend;
	struct videobuf_queue      dvbq;

	/* video-buf-dvb state info */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,15)
	struct mutex               lock;
#else
	struct semaphore           lock;
#endif
	struct task_struct         *thread;
	int                        nfeeds;

	/* videobuf_dvb_(un)register manges this */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12))
	struct dvb_adapter         adapter;
#else
	struct dvb_adapter         *adapter;
#endif
	struct dvb_demux           demux;
	struct dmxdev              dmxdev;
	struct dmx_frontend        fe_hw;
	struct dmx_frontend        fe_mem;
	struct dvb_net             net;
};

int videobuf_dvb_register(struct videobuf_dvb *dvb,
			  struct module *module,
			  void *adapter_priv,
			  struct device *device);
void videobuf_dvb_unregister(struct videobuf_dvb *dvb);

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
