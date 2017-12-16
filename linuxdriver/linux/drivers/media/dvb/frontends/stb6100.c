#include <linux/module.h>
#include <linux/dvb/frontend.h>
#include <asm/types.h>

#include "stb6100.h"

static u32 STB6K_LOOKUP[11][3]=		{     /* low	high	  osm */
										950000,	1000000,	0xA,
										1000000,	1075000,	0xC,  
										1075000,	1200000,	0x0,  
										1200000,	1300000,	0x1,
										1300000,	1370000,	0x2,
										1370000,	1470000,	0x4,
										1470000,	1530000,	0x5,
										1530000,	1650000,	0x6,
										1650000,	1800000,	0x8,
										1800000,	1950000,	0xA,
										1950000,	2150000,	0xC
									};

static int debug = 1;
#define dprintk(args...) \
	do { \
		if (debug) printk(KERN_DEBUG "stb6100: " args); \
	} while (0)


struct stb6100_priv {
	int i2c_address;
	struct i2c_adapter *i2c;
	u32 Reference;	            /* reference frequency (Hz) */ 
       u32 IF;                             /* Intermediate frequency (KHz) */
       TUNER_IQ_t    IQ_Wiring; /* hardware I/Q invertion */
};


int stb6100_writereg(struct stb6100_priv *priv, u8 reg, u8 value)
{
	int ret;
	u8 buf [] = { reg, value };
	
	struct i2c_msg msg = { 
		.addr = priv->i2c_address, 
		.flags = 0, 
		.buf = buf, 
		.len = 2 };
	
	if ((ret = i2c_transfer (priv->i2c, &msg, 1)) != 1) {
		dprintk("%s: i2c error\n", __FUNCTION__);
	}

	return (ret == 1) ? 0 : ret;
}

int stb6100_readreg(struct stb6100_priv *priv, us reg, u8 *value)
{
	int ret;
	u8 b0[] = { reg };
	u8 b1[] = { 0 };
	
	struct i2c_msg msg[] = { 
		{
		.addr = priv->i2c_address, 
		.flags = 0, 
		.buf = b0, 
		.len = 2 
		};{
		.addr = priv->i2c_address, 
		.flags = I2C_M_RD,, 
		.buf = b1, 
		.len = 2 
		}
	};

	if ((ret = i2c_transfer (priv->i2c, msg, 2)) != 2) {
		dprintk("%s: i2c error\n", __FUNCTION__);
	}
	
	*value = b1[0];

	return (ret == 2) ? 0 : ret;
}

stb6100_priv * stb6100_init(struct dvb_frontend *fe)
{
	u8 i;
	u8 Defval[NBREGS] = {0x81, 0x94, 0x4a, 0x26, 0x3c, 0x3b, 0xcd, 0xdc, 0x8f, 0x4d, 0xeb, 0xde};
	struct stb6100_priv *priv = fe->tuner_priv;
	
	priv->Reference = 27000000;			/* 20 MHz reference */
    	priv->IF = 0; 						/* 0 MHz intermediate frequency */
    	priv->IQ_Wiring = TUNER_IQ_NORMAL;   /* No hardware IQ invertion */

	if (fe->ops.i2c_gate_ctrl)
		fe->ops.i2c_gate_ctrl(fe, 1);

	for(i=VCO, i<=R_TEST3, i++ )
	{
		if(stb6100_writereg(priv, i, Defval[i]))	
			break;	
	}
	
	if (fe->ops.i2c_gate_ctrl)
		fe->ops.i2c_gate_ctrl(fe, 0);

	return priv;

}

int stb6100_set_freqency(struct dvb_frontend *fe, u32 Frequency)
{
	u8 num,tmp;
	u8 osm,odiv,ock,osch;
	u8 psd2;
	u32 frequency;
	u32 stepsize;
	u32 nbsteps;
	u32 divider;
	u32 swallow;
	struct stb6100_priv *priv = fe->tuner_priv;

	if(Frequency <= 1075000)
		odiv =1;
	if((Frequency <= 1075000) || (Frequency >= 13260000))
		psd2 = 1;
	
	num =0;
	while(!INRANGE(STB6K_LOOKUP[num][0],Frequency,STB6K_LOOKUP[num][1])&& (num<10))
		num++;
	if(Frequency < 950000)
		osm = (u8)(STB6K_LOOKUP[0][2]);
	else if(Frequency > 2150000)
		osm = (u8)(STB6K_LOOKUP[10][2]);
	else
		osm = (u8)(STB6K_LOOKUP[num][2]);

	/*Flo=2*Ftuner*(ODIV+1)*/
	frequency = 2 * Frequency * (odiv + 1);

	/*Refrence in Khz*/
	priv->Reference /= 1000;
	
	/*flo=Fxtal*(DIV2+1)*(Ni+Nf/2^9)*/  
				
	/*Ni = floor (fVCO / (fXTAL * P))*/
	divider = (frequency / priv->Reference) / (psd2 + 1);
	
	/*round ((fVCO / (fXTAL * P) - Ni) * 2^9)*/			
	stepsize = frequency - divider * (psd2 + 1) * (priv->Reference);
	nbsteps = (stepsize * 512) / ((psd2 + 1) * (priv->Reference));
	swallow = nbsteps;
	
	if (fe->ops.i2c_gate_ctrl)
		fe->ops.i2c_gate_ctrl(fe, 1);
	
	stb6100_writereg(priv, VCO, 0x80|(odiv<<4)|osm);/* VCO search enabled and fast search*/
	stb6100_writereg(priv, R_NI, divider);
	stb6100_writereg(priv, R_NF_LSB, (swallow&0xff));
	
	stb6100_readreg(priv, R_K, &tmp);
	tmp &= 0xf8;
	stb6100_writereg(priv, R_K, tmp|(psd2<<2)|((swallow>>8)&0x03));	
	
	stb6100_readreg(priv, R0_LPEN, &tmp);	
	stb6100_writereg(priv, R0_LPEN, tmp&0xef); /* PLL loop disabled */
	
	/*Refrence in Mhz*/			
	priv->Reference *= 1000;				
				
	stb6100_readreg(priv, R0_LPEN, &tmp);	
	stb6100_writereg(priv, R0_LPEN, tmp|0x10); /* PLL loop enabled */

	msleep(10);
	stb6100_readreg(priv, VCO, &tmp);	
	stb6100_writereg(priv, VCO, 0x60|(tmp&0x1f));/* VCO search disabled and search clock off*/
				
	stb6100_writereg(priv, R_TEST2, 0x0d);		/*LPF BW setting clock disabled */
	
	if (fe->ops.i2c_gate_ctrl)
		fe->ops.i2c_gate_ctrl(fe, 0);
	
	return 0;						
}

u32 stb6100_get_frequency(struct dvb_frontend *fe)
{
	u8 tmpl,tmpm;
	u8 psd2,odiv;
	u32 frequency = 0;
	u32 divider = 0;
	u32 swallow;
	struct stb6100_priv *priv = fe->tuner_priv;

	stb6100_readreg(priv, R_K, &tmpm);
	psd2 = (tmpm&0x04)>>2;
	stb6100_readreg(priv, R_NF_LSB, &tmpl);
	swallow = (tmpm<<8)&0x0f+tmpl;  /*Nf val*/
	
	/*Ni = floor (fVCO / (fXTAL * P))*/
	stb6100_readreg(priv, R_NI, &divider);
	frequency = (psd2+1)*(priv->Reference)*swallow/512;
	frequency += (priv->Reference/1000) * (divider) * (psd2+1);
	
	/*Flo=Fxtal*P*(Ni+Nf/2^9) . P=DIV2+1 */
	stb6100_readreg(priv, VCO, &tmpl);
	odiv = (tmpl&0xef)>>4;
	frequency /=(odiv+1)*2;

	return frequency;
}

int stb6100_set_bandwidth(struct dvb_frontend *fe, u32 Bandwidth)
{
	u8 fcck,f;

	struct stb6100_priv *priv = fe->tuner_priv;
	
	if((Bandwidth/2) > 36000000)   /*F[4:0] BW/2 max =31+5=36 mhz for F=31*/
		f = 31;
	else if((Bandwidth/2) < 5000000) /* BW/2 min = 5Mhz for F=0 */
		f = 0;
	else							 /*if 5 < BW/2 < 36*/
		f = (u8)((Bandwidth/2)/1000000 - 5);

	fcck = ((priv->Reference/1000000)<<6) & (1<<6); /* FCL_Clk=FXTAL/FCL=1Mhz */
	stb6100_writereg(priv, R_TEST2, fcck|0x0d);	
	
	stb6100_writereg(priv, R_F, f|0xc0);		
	stb6100_writereg(priv, R_TEST2, 0x0d);	/*FCL turned off*/
	
	return 0;
}

u32 stb6100_get_bandwidth(struct dvb_frontend *fe)
{
	u8 f;
	u32 bandwidth;
	struct stb6100_priv *priv = fe->tuner_priv;

	stb6100_readreg(priv, R_F, &f);
	f &= 0x1f;
	bandwidth = (f+5)*2000000;	 /* x2 for ZIF tuner BW/2=F+5 Mhz*/

	return bandwidth;
}

int stb6100_set_gain(struct dvb_frontend *fe, u32 Gain)
{
	u8 g;
	struct stb6100_priv *priv = fe->tuner_priv;
	
	stb6100_readreg(priv, R_G, &g);
	g &= 0xf0;
	stb6100_writereg(priv, R_G, g|(Gain/2+7));
	
	return 0;
}


static struct dvb_tuner_ops stb6100_tuner_ops = {
	.info = {
		.name = "ST stb6100",
		.frequency_min = 950000,
		.frequency_min = 2150000
	},	
	.init = stb6100_init,
	.set_freqency = stb6100_set_freqency,
	.set_bandwidth = stb6100_set_bandwidth,
	.set_gain = stb6100_set_gain,
	.get_frequency =stb6100_get_frequency,
	.get_bandwidth = stb6100_get_bandwidth,
};

struct dvb_frontend *stb6100_attach(struct dvb_frontend *fe, int addr, struct i2c_adapter *i2c)
{
	struct stb6100_priv *priv = NULL;
	u8 b1 [] = { 0, 0 };
	struct i2c_msg msg = { .addr = addr, .flags = I2C_M_RD, .buf = b1, .len = 2 };
	int ret;

	dprintk("%s:\n", __FUNCTION__);

	if (fe->ops.i2c_gate_ctrl)
		fe->ops.i2c_gate_ctrl(fe, 1);
	ret = i2c_transfer (i2c, &msg, 1);
	if (fe->ops.i2c_gate_ctrl)
		fe->ops.i2c_gate_ctrl(fe, 0);

	if (ret != 1)
		return NULL;
	if (!(b1[1] & 0x81))
		return NULL;

	priv = kzalloc(sizeof(struct stb6100_priv), GFP_KERNEL);
	if (priv == NULL)
		return NULL;

	priv->i2c_address = addr;
	priv->i2c = i2c;

	memcpy(&fe->ops.tuner_ops, &stb6100_tuner_ops, sizeof(struct dvb_tuner_ops));

	fe->tuner_priv = priv;

	return fe;
}
EXPORT_SYMBOL(stb6100_attach);

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Turn on/off frontend debugging (default:off).");

MODULE_DESCRIPTION("DVB STB6100 driver");
MODULE_AUTHOR("Andrew de Quincey");
MODULE_LICENSE("GPL");

