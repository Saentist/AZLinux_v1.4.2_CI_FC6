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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "dvb_frontend.h"
#include "mb86a16.h"

unsigned int verbose = 1;
module_param(verbose, int, 0644);

#define ABS(x)		((x) < 0 ? (-x) : (x))

struct mb86a16_state {
	struct i2c_adapter		*i2c_adap;
	const struct mb86a16_config	*config;
	struct dvb_frontend		frontend;
	u8				signal;
	fe_sec_mini_cmd_t minicmd;	
	fe_sec_tone_mode_t tone;
};

unsigned char tmd[5] = { 0, 0, 0xff, 0xff, 0xff }; // Tone signal output signal

#define MB86A16_ERROR		0
#define MB86A16_NOTICE		1
#define MB86A16_INFO		2
#define MB86A16_DEBUG		3

#define dprintk(x, y, z, format, arg...) do {						\
	if (z) {									\
		if	((x > MB86A16_ERROR) && (x > y))				\
			printk(KERN_ERR "%s: " format "\n", __func__, ##arg);		\
		else if ((x > MB86A16_NOTICE) && (x > y))				\
			printk(KERN_NOTICE "%s: " format "\n", __func__, ##arg);	\
		else if ((x > MB86A16_INFO) && (x > y))					\
			printk(KERN_INFO "%s: " format "\n", __func__, ##arg);		\
		else if ((x > MB86A16_DEBUG) && (x > y))				\
			printk(KERN_DEBUG "%s: " format "\n", __func__, ##arg);		\
	} else {									\
		if (x > y)								\
			printk(format, ##arg);						\
	}										\
} while (0)

#define TRACE_IN 
#define TRACE_OUT 

static int mb86a16_write(struct mb86a16_state *state, u8 reg, u8 val)
{
	int ret;
	u8 buf[] = { reg, val };

	struct i2c_msg msg = {
		.addr = state->config->demod_address,
		.flags = 0,
		.buf = buf,
		.len = 2
	};

	dprintk(verbose, MB86A16_DEBUG, 1,
		"writing to [0x%02x],Reg[0x%02x],Data[0x%02x]",
		state->config->demod_address, buf[0], buf[1]);

	ret = i2c_transfer(state->i2c_adap, &msg, 1);
	if (ret != 1) {
		dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer Error");
		return -EREMOTEIO;
	} else {
		//dprintk(verbose, MB86A16_DEBUG, 1, "I2C transfer successful");
		return ret;
	}

	return -EIO;
}

static int mb86a16_read(struct mb86a16_state *state, u8 reg, u8 *val)
{
	int ret;
	u8 b0[] = { reg };
	u8 b1[] = { 0 };

	struct i2c_msg msg[] = {
		{
			.addr = state->config->demod_address,
			.flags = 0,
			.buf = b0,
			.len = 1
		},{
			.addr = state->config->demod_address,
			.flags = I2C_M_RD,
			.buf = b1,
			.len = 1
		}
	};
	ret = i2c_transfer(state->i2c_adap, msg, 2);
	if (ret != 2) {
		dprintk(verbose, MB86A16_ERROR, 1, "read error(reg=0x%02x, ret=0x%i)",
			reg, ret);

		return -EREMOTEIO;
	}
	*val = b1[0];

	return ret;
}

// Antenna Set, DiSEqC signal setting
static int mb86a16_send_diseqc_msg(struct dvb_frontend* fe, struct dvb_diseqc_master_cmd *m)
{
	struct mb86a16_state* state = fe->demodulator_priv;
	u8 diseqc_reg;
	unsigned char i=0;

	// write 1 to prepare outputs diseqc signal
	if (mb86a16_write(state, 0x16, 0x80) != 1)
		goto err;	
	// DiSEQc mode off, output close    
	if (mb86a16_write(state, 0x1e, 0) != 1)
		goto err;
	// TONE signal setting #2, tone output is fix low
	if (mb86a16_write(state, 0x20, 0x04) != 1)
		goto err;

	msleep_interruptible(10);
	
	diseqc_reg = 0x18;
	if(m->msg_len >5 || m->msg_len <3) 
		return -1;	
	for (i=0; i<m->msg_len; i++) {	
		if (mb86a16_write(state, diseqc_reg, m->msg[i]) != 1)
			goto err;	
		diseqc_reg++;	
	}
	i+=0x90;
	msleep(10);
	
	if (mb86a16_write(state, 0x16, i) != 1)
		goto err;
	if (mb86a16_write(state, 0x1e, 0x1) != 1)
		goto err;
	
	return 0;
	
err:	
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");	
	return -EREMOTEIO;
}

static int mb86a16_send_diseqc_burst (struct dvb_frontend* fe, fe_sec_mini_cmd_t burst)
{	
	struct mb86a16_state* state = fe->demodulator_priv;	
		
	msleep(100);	
	state->minicmd = burst;	
	switch (burst) {	
		case SEC_MINI_A:
			dprintk(verbose, MB86A16_DEBUG, 1, "SEC_MINI_A");
			if (mb86a16_write(state, 0x16, 0x98) != 1)
		 		goto err;
			if (mb86a16_write(state, 0x1e, 0x01) != 1)
		 		goto err;	
			break;	
		case SEC_MINI_B:
			dprintk(verbose, MB86A16_DEBUG, 1, "SEC_MINI_B");
			if (mb86a16_write(state, 0x16, 0x90) != 1)
		 		goto err;
			if (mb86a16_write(state, 0x1e, 0x01) != 1)
		 		goto err;		
			break;	
	}	
	return 0;
	
err:	
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");	
	return -EREMOTEIO;
}

// Antenna set, TONE signal Setting
static int mb86a16_send_tone_msg(struct dvb_frontend* fe, fe_sec_tone_mode_t tone)
{
	struct mb86a16_state* state = fe->demodulator_priv;
	
	state->tone = tone;	
	msleep(100);	
	switch (tone) {	
		case SEC_TONE_ON:
			dprintk(verbose, MB86A16_DEBUG, 1, "SEC_TONE_ON");	
			if (mb86a16_write(state, 0x20, 0x00) != 1)
				goto err;
			if (mb86a16_write(state, 0x16, 0xa0) != 1)
				goto err;	
			if (mb86a16_write(state, 0x1e, 0x01) != 1)
				goto err;
			break;	
		case SEC_TONE_OFF:
			dprintk(verbose, MB86A16_DEBUG, 1, "SEC_TONE_OFF");	
			if (mb86a16_write(state, 0x20, 0x04) != 1)
				goto err;
			if (mb86a16_write(state, 0x16, 0x80) != 1)
				goto err;	
			if (mb86a16_write(state, 0x1e, 0x00) != 1)
				goto err;	
			break;	
		default:		
			return -EINVAL;	
	}	
	return 0;
	
err:	
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");	
	return -EREMOTEIO;
}


static int CNTM_set(struct mb86a16_state *state,
		    unsigned char timint1,
		    unsigned char timint2,
		    unsigned char cnext)
{
	unsigned char val;

	val = (timint1 << 4) | (timint2 << 2) | cnext;
	if (mb86a16_write(state, 0x36, val) != 1)
		goto err;

	return 0;

err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int smrt_set(struct mb86a16_state *state,
		    int sr,
		    int DECI,
		    int CSEL,
		    int RSEL,
		    int clkmst)
{
	int tmp ;
	int m ;
	unsigned char STOFS0, STOFS1;
	int ack = 1 ;

	m = 1 << DECI;
	tmp = (8192 * clkmst - 2 * m * sr * 8192 + clkmst / 2) / clkmst;

	STOFS0 = tmp & 0x0ff;
	STOFS1 = (tmp & 0xf00) >> 8;

	if (mb86a16_write(state, 0x03, (DECI << 2) | (CSEL << 1) | RSEL) != 1)
		ack = 0;
	if (mb86a16_write(state, 0x04, STOFS0) != 1)
		ack = 0;
	if (mb86a16_write(state, 0x05, STOFS1) != 1)
		ack = 0;

	if (ack == 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
		return -1;
	}

	return 0;
}

static int srst(struct mb86a16_state *state)
{
	if (mb86a16_write(state, 0x0c, 0x04) != 1)
		goto err;

	return 0;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;

}

static int afcex_data_set(struct mb86a16_state *state,
			  unsigned char AFCEX_L,
			  unsigned char AFCEX_H)
{
	if (mb86a16_write(state, 0x2b, AFCEX_L) != 1)
		goto err;
	if (mb86a16_write(state, 0x2c, AFCEX_H) != 1)
		goto err;

	return 0;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");

	return -1;
}

static int afcofs_data_set(struct mb86a16_state *state,
			   unsigned char AFCEX_L,
			   unsigned char AFCEX_H)
{
	if (mb86a16_write(state, 0x58, AFCEX_L) != 1)
		goto err;

	if (mb86a16_write(state, 0x59, AFCEX_H) != 1)
		goto err;

	return 0;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int stlp_set(struct mb86a16_state *state,
		    unsigned char STRAS,
		    unsigned char STRBS)
{
	if (mb86a16_write(state, 0x0a, (STRBS << 3) | (STRAS)) != 1)
		goto err;

	return 0;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int Vi_set(struct mb86a16_state *state, unsigned char ETH, unsigned char VIA)
{
	// ETH = 4
	if (mb86a16_write(state, 0x3b, 0x04) != 1)
		goto err;
	// VIA = 5
	if (mb86a16_write(state, 0x3c, 0xf5) != 1)
		goto err;

	return 0;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int initial_set(struct mb86a16_state *state)
{
	//int i;
	//printk("%s: Initializing ..\n", __func__);
	//printk(KERN_ERR "Initializing testing \n");
	//for(i=0; i<452; i++)
	//{
	//	mb86a16_write(state, 0x08, 0x16);
	//}
	//printk(KERN_ERR "Initializing testing \n");
	// Timing recovery		
	if (stlp_set(state, 5, 7))
		goto err;
	if (afcex_data_set(state, 0, 0))
		goto err;
	if (afcofs_data_set(state, 0, 0))
		goto err;

	// CRBS = 2, CRAS = 6
	if (mb86a16_write(state, 0x08, 0x16) != 1)
		goto err;

	// CRWS = 1 // was 2 (0x22)
	if (mb86a16_write(state, 0x2f, 0x21) != 1)
		goto err;

	// VMAG = 7
	if (mb86a16_write(state, 0x39, 0x38) != 1)
		goto err;

	// Front AGC Setting 1
	if (mb86a16_write(state, 0x3d, 0x00) != 1)
		goto err;
	// Front AGC Setting 2(4 to 3)
	if (mb86a16_write(state, 0x3e, 0x1c) != 1)
		goto err;
	// Front AGC Setting 3(3 to 2)
	if (mb86a16_write(state, 0x3f, 0x20) != 1)
		goto err;
	// Front AGC Setting 4(3 to 4)
	if (mb86a16_write(state, 0x40, 0x1e) != 1)
		goto err;
	//Front AGC Setting 5(2 to 3)
	if (mb86a16_write(state, 0x41, 0x23) != 1)
		goto err;
	if (mb86a16_write(state, 0x54, 0xff) != 1)
		goto err;
	// TS Output
	if (mb86a16_write(state, 0x00, 0x00) != 1)
		goto err;

	return 0;

err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int S01T_set(struct mb86a16_state *state,
		    unsigned char s1t,
		    unsigned s0t)
{
	if (mb86a16_write(state, 0x33, (s1t << 3) | s0t) != 1)
		goto err;

	return 0;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int EN_set(struct mb86a16_state *state,
		  int cren,
		  int afcen)
{
	unsigned char val;

	val = 0x7a | (cren << 7) | (afcen << 2);
	if (mb86a16_write(state, 0x49, val) != 1)
		goto err;

	return 0;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int AFCEXEN_set(struct mb86a16_state *state,
		       int afcexen,
		       int smrt)
{
	unsigned char AFCA ;

	if (smrt > 18875)
		AFCA = 4;
	else if (smrt > 9375)
		AFCA = 3;
	else if (smrt > 2250)
		AFCA = 2;
	else
		AFCA = 1;

	if (mb86a16_write(state, 0x2a, 0x02 | (afcexen << 5) | (AFCA << 2)) != 1)
		goto err;

	return 0;

err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int DAGC_data_set(struct mb86a16_state *state,
			 unsigned char DAGCA,
			 unsigned char DAGCW)
{
	if (mb86a16_write(state, 0x2d, (DAGCA << 3) | DAGCW) != 1)
		goto err;

	return 0;

err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static void smrt_info_get(struct mb86a16_state *state,
			  int sr,
			  unsigned char *DECI,
			  unsigned char *CSEL,
			  unsigned char *RSEL,
			  int *clkmst)
{
	if (sr >= 37501) {
		*DECI = 0; *CSEL = 0; *RSEL = 0;
	} else if (sr >= 30001) {
		*DECI = 0; *CSEL = 0; *RSEL = 1;
	} else if (sr >= 26251) {
		*DECI = 0; *CSEL = 1; *RSEL = 0;
	} else if (sr >= 22501) {
		*DECI = 0; *CSEL = 1; *RSEL = 1;
	} else if (sr >= 18751) {
		*DECI = 1; *CSEL = 0; *RSEL = 0;
	} else if (sr >= 15001) {
		*DECI = 1; *CSEL = 0; *RSEL = 1;
	} else if (sr >= 13126) {
		*DECI = 1; *CSEL = 1; *RSEL = 0;
	} else if (sr >= 11251) {
		*DECI = 1; *CSEL = 1; *RSEL = 1;
	} else if (sr >= 9376) {
		*DECI = 2; *CSEL = 0; *RSEL = 0;
	} else if (sr >= 7501) {
		*DECI = 2; *CSEL = 0; *RSEL = 1;
	} else if (sr >= 6563) {
		*DECI = 2; *CSEL = 1; *RSEL = 0;
	} else if (sr >= 5626) {
		*DECI = 2; *CSEL = 1; *RSEL = 1;
	} else if (sr >= 4688) {
		*DECI = 3; *CSEL = 0; *RSEL = 0;
	} else if (sr >= 3751) {
		*DECI = 3; *CSEL = 0; *RSEL = 1;
	} else if (sr >= 3282) {
		*DECI = 3; *CSEL = 1; *RSEL = 0;
	} else if (sr >= 2814) {
		*DECI = 3; *CSEL = 1; *RSEL = 1;
	} else if (sr >= 2344) {
		*DECI = 4; *CSEL = 0; *RSEL = 0;
	} else if (sr >= 1876) {
		*DECI = 4; *CSEL = 0; *RSEL = 1;
	} else if (sr >= 1641) {
		*DECI = 4; *CSEL = 1; *RSEL = 0;
	} else if (sr >= 1407) {
		*DECI = 4; *CSEL = 1; *RSEL = 1;
	} else if (sr >= 1172) {
		*DECI = 5; *CSEL = 0; *RSEL = 0;
	} else if (sr >=  939) {
		*DECI = 5; *CSEL = 0; *RSEL = 1;
	} else if (sr >=  821) {
		*DECI = 5; *CSEL = 1; *RSEL = 0;
	} else {
		*DECI = 5; *CSEL = 1; *RSEL = 1;
	}

	if (*CSEL == 0)
		*clkmst = 92000;
	else
		*clkmst = 61333;

}

static int signal_det(struct mb86a16_state *state,
		      int smrt, unsigned char *DECI,
		      unsigned char *CSEL,
		      unsigned char *RSEL,
		      int *clkmst,
		      unsigned char *SIG)
{

	int ret ;
	int smrtd ;
	int wait_sym ;
	int wait_t ;
	unsigned char S[3] ;
	int i ;

	if (*SIG > 45) {
		if (CNTM_set(state, 2, 1, 2) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "CNTM set Error");
			return -1;
		}
		wait_sym = 40000;
	} else {
		if (CNTM_set(state, 3, 1, 2) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "CNTM set Error");
			return -1;
		}
		wait_sym = 80000;
	}
	for (i = 0; i < 3; i++) {
		if (i == 0 )
			smrtd = smrt * 98 / 100;
		else if (i == 1)
			smrtd = smrt;
		else
			smrtd = smrt * 102 / 100;
		smrt_info_get(state, smrtd, DECI, CSEL, RSEL, clkmst);
		smrt_set(state, smrtd, *DECI, *CSEL, *RSEL, *clkmst);
		srst(state);
		wait_t = (wait_sym + 99 * smrtd / 100) / smrtd;
		if (wait_t == 0)
			wait_t = 1;
		msleep_interruptible(10);
		if (mb86a16_read(state, 0x37, &(S[i])) != 2) {
			dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
			return -EREMOTEIO;
		}
	}
	if ((S[1] > S[0] * 112 / 100) &&
	    (S[1] > S[2] * 112 / 100)) {

		ret = 1;
	} else {
		ret = 0;
	}
	*SIG = S[1];

	if (CNTM_set(state, 0, 1, 2) < 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "CNTM set Error");
		return -1;
	}

	return ret;
}

static int rf_val_set(struct mb86a16_state *state,
		      int f,
		      int smrt,
		      unsigned char R)
{
	unsigned char C, F, B;
	int M;
	unsigned char rf_val[5];
	int ack = -1;

	if (smrt > 37750 )
		C = 1;
	else if (smrt > 18875)
		C = 2;
	else if (smrt > 5500 )
		C = 3;
	else
		C = 4;

	if (smrt > 30500)
		F = 3;
	else if (smrt > 9375)
		F = 1;
	else if (smrt > 4625)
		F = 0;
	else
		F = 2;

	if (f < 1060)
		B = 0;
	else if (f < 1175)
		B = 1;
	else if (f < 1305)
		B = 2;
	else if (f < 1435)
		B = 3;
	else if (f < 1570)
		B = 4;
	else if (f < 1715)
		B = 5;
	else if (f < 1845)
		B = 6;
	else if (f < 1980)
		B = 7;
	else if (f < 2080)
		B = 8;
	else
		B = 9;

	M = f * (1 << R) / 2;

	rf_val[0] = 0x01 | (C << 3) | (F << 1);
	rf_val[1] = (R << 5) | ((M & 0x1f000) >> 12);
	rf_val[2] = (M & 0x00ff0) >> 4;
	rf_val[3] = ((M & 0x0000f) << 4) | B;

	// Frequency Setting #1 #2 #3 #4 and frequency Setting Switch
	if (mb86a16_write(state, 0x21, rf_val[0]) != 1)
		ack = 0;
	if (mb86a16_write(state, 0x22, rf_val[1]) != 1)
		ack = 0;
	if (mb86a16_write(state, 0x23, rf_val[2]) != 1)
		ack = 0;
	if (mb86a16_write(state, 0x24, rf_val[3]) != 1)
		ack = 0;
	if (mb86a16_write(state, 0x25, 0x01) != 1)
		ack = 0;
	if (ack == 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "rf_val_set - I2C transfer error");
		return -EREMOTEIO;
	}

	return 0;
}

static int afcerr_chk(struct mb86a16_state *state,
		      int clkmst)
{
	unsigned char AFCM_L, AFCM_H ;
	int AFCM ;
	int afcm, afcerr ;

	if (mb86a16_read(state, 0x0e, &AFCM_L) != 2)
		goto err;
	if (mb86a16_read(state, 0x0f, &AFCM_H) != 2)
		goto err;

	AFCM = (AFCM_H << 8) + AFCM_L;

	if (AFCM > 2048)
		afcm = AFCM - 4096;
	else
		afcm = AFCM;
	afcerr = afcm * clkmst / 8192;

	return afcerr;

err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int dagcm_val_get(struct mb86a16_state *state)
{
	int DAGCM;
	unsigned char DAGCM_H, DAGCM_L;

	if (mb86a16_read(state, 0x45, &DAGCM_L) != 2)
		goto err;
	if (mb86a16_read(state, 0x46, &DAGCM_H) != 2)
		goto err;

	DAGCM = (DAGCM_H << 8) + DAGCM_L;

	return DAGCM;

err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int mb86a16_read_status(struct dvb_frontend *fe, fe_status_t *status)
{
	struct mb86a16_state *state = fe->demodulator_priv;

	*status = 0;
	if (mb86a16_read(state, 0x0d, &state->signal) != 2)
		goto err;

	if (state->signal & 0x02)
		*status |= FE_HAS_VITERBI;
	if (state->signal & 0x01)
		*status |= FE_HAS_SYNC;
	if (state->signal & 0x03)
	{
		*status |= FE_HAS_SIGNAL;
		*status |= FE_HAS_LOCK;
		*status |= FE_HAS_CARRIER;
	}
	return 0;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static int sync_chk(struct mb86a16_state *state,
		    unsigned char *VIRM)
{
	int sync ;

	if (mb86a16_read(state, 0x0d, &state->signal) != 2)
		goto err;

	dprintk(verbose, MB86A16_INFO, 1, "Status = %02x,", state->signal);
	sync = state->signal & 0x01;
	*VIRM = (state->signal & 0x1c) >> 2;

	return sync;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;

}

static int freqerr_chk(struct mb86a16_state *state,
		       int fTP,
		       int smrt,
		       int unit)
{
	unsigned char CRM, AFCML, AFCMH;
	unsigned char temp1, temp2, temp3;
	unsigned char DECI, CSEL, RSEL;
	int clkmst;
	int crm, afcm, AFCM;
	int crrerr, afcerr;		// [kHz]
	int frqerr;			// [MHz]
	int afcen, afcexen = 0;
	int R, M, fOSC, fOSC_OFS;

	if (mb86a16_read(state, 0x43, &CRM) != 2)
		goto err;

	if (CRM > 127)
		crm = CRM - 256;
	else
		crm = CRM;

	crrerr = smrt * crm / 256;
	if (mb86a16_read(state, 0x49, &temp1) != 2)
		goto err;

	afcen = (temp1 & 0x04) >> 2;
	if (afcen == 0) {
		if (mb86a16_read(state, 0x2a, &temp1) != 2)
			goto err;
		afcexen = (temp1 & 0x20) >> 5;
	}

	if (afcen == 1) {
		if (mb86a16_read(state, 0x0e, &AFCML) != 2)
			goto err;
		if (mb86a16_read(state, 0x0f, &AFCMH) != 2)
			goto err;
	} else if (afcexen == 1) {
		if (mb86a16_read(state, 0x2b, &AFCML) != 2)
			goto err;
		if (mb86a16_read(state, 0x2c, &AFCMH) != 2)
			goto err;
	}
	if ((afcen == 1) || (afcexen == 1)) {
		smrt_info_get(state, smrt, &DECI, &CSEL, &RSEL, &clkmst);
		AFCM = ((AFCMH & 0x01) << 8) + AFCML;
		if (AFCM > 255)
			afcm = AFCM - 512;
		else
			afcm = AFCM;

		afcerr = afcm * clkmst / 8192;
	} else
		afcerr = 0;

	if (mb86a16_read(state, 0x22, &temp1) != 2)
		goto err;
	if (mb86a16_read(state, 0x23, &temp2) != 2)
		goto err;
	if (mb86a16_read(state, 0x24, &temp3) != 2)
		goto err;

	R = (temp1 & 0xe0) >> 5;
	M = ((temp1 & 0x1f) << 12) + (temp2 << 4) + (temp3 >> 4);
	if (R == 0)
		fOSC = 2 * M;
	else
		fOSC = M;

	fOSC_OFS = fOSC - fTP;

	if (unit == 0) {	//[MHz]
		if (crrerr + afcerr + fOSC_OFS * 1000 >= 0)
			frqerr = (crrerr + afcerr + fOSC_OFS * 1000 + 500) / 1000;
		else
			frqerr = (crrerr + afcerr + fOSC_OFS * 1000 - 500) / 1000;
	} else {	//[kHz]
		frqerr = crrerr + afcerr + fOSC_OFS * 1000;
	}

	return frqerr;
err:
	dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
	return -EREMOTEIO;
}

static unsigned char vco_dev_get(struct mb86a16_state *state, int smrt)
{
	unsigned char R;

	if (smrt > 9375)
		R = 0;
	else
		R = 1;

	return R;
}

static void swp_info_get(struct mb86a16_state *state,
			 int fOSC_start,
			 int smrt, int clkmst,
			 int v, int R,
			 int swp_ofs,
			 int *fOSC,
			 int *afcex_freq,
			 unsigned char *AFCEX_L,
			 unsigned char *AFCEX_H)
{
	int AFCEX ;
	int crnt_swp_freq ;

	crnt_swp_freq = fOSC_start * 1000 + v * swp_ofs;

	if (R == 0 )
		*fOSC = (crnt_swp_freq + 1000) / 2000 * 2;
	else
		*fOSC = (crnt_swp_freq + 500) / 1000;

	if (*fOSC >= crnt_swp_freq)
		*afcex_freq = *fOSC *1000 - crnt_swp_freq;
	else
		*afcex_freq = crnt_swp_freq - *fOSC * 1000;

	AFCEX = *afcex_freq * 8192 / clkmst;
	*AFCEX_L =  AFCEX & 0x00ff;
	*AFCEX_H = (AFCEX & 0x0f00) >> 8;
}


static int swp_freq_calcuation(struct mb86a16_state *state, int i, int v, int *V,  int vmax, int vmin,
			       int SIGMIN, int fOSC, int afcex_freq, int swp_ofs, unsigned char *SIG1)
{
	int swp_freq ;

	if ((i % 2 == 1) && (v <= vmax)) {
		// positive v (case 1)
		if ((v - 1 == vmin)				&&
		    (*(V + 30 + v) >= 0)			&&
		    (*(V + 30 + v - 1) >= 0)			&&
		    (*(V + 30 + v - 1) > *(V + 30 + v))		&&
		    (*(V + 30 + v - 1) > SIGMIN)) {

			swp_freq = fOSC * 1000 + afcex_freq - swp_ofs;
			*SIG1 = *(V + 30 + v - 1);
		} else if ((v == vmax)				&&
			   (*(V + 30 + v) >= 0)			&&
			   (*(V + 30 + v - 1) >= 0)		&&
			   (*(V + 30 + v) > *(V + 30 + v - 1))	&&
			   (*(V + 30 + v) > SIGMIN)) {
			// (case 2)
			swp_freq = fOSC * 1000 + afcex_freq;
			*SIG1 = *(V + 30 + v);
		} else if ((*(V + 30 + v) > 0)			&&
			   (*(V + 30 + v - 1) > 0)		&&
			   (*(V + 30 + v - 2) > 0)		&&
			   (*(V + 30 + v - 3) > 0)		&&
			   (*(V + 30 + v - 1) > *(V + 30 + v))	&&
			   (*(V + 30 + v - 2) > *(V + 30 + v - 3)) &&
			   ((*(V + 30 + v - 1) > SIGMIN)	||
			   (*(V + 30 + v - 2) > SIGMIN))) {
			// (case 3)
			if (*(V + 30 + v - 1) >= *(V + 30 + v - 2)) {
				swp_freq = fOSC * 1000 + afcex_freq - swp_ofs;
				*SIG1 = *(V + 30 + v - 1);
			} else {
				swp_freq = fOSC * 1000 + afcex_freq - swp_ofs * 2;
				*SIG1 = *(V + 30 + v - 2);
			}
		} else if ((v == vmax)				&&
			   (*(V + 30 + v) >= 0)			&&
			   (*(V + 30 + v - 1) >= 0)		&&
			   (*(V + 30 + v - 2) >= 0)		&&
			   (*(V + 30 + v) > *(V + 30 + v - 2))	&&
			   (*(V + 30 + v - 1) > *(V + 30 + v - 2)) &&
			   ((*(V + 30 + v) > SIGMIN)		||
			   (*(V + 30 + v - 1) > SIGMIN))) {
			// (case 4)
			if (*(V + 30 + v) >= *(V + 30 + v - 1)) {
				swp_freq = fOSC * 1000 + afcex_freq;
				*SIG1 = *(V + 30 + v);
			} else {
				swp_freq = fOSC * 1000 + afcex_freq - swp_ofs;
				*SIG1 = *(V + 30 + v - 1);
			}
		} else  {
			swp_freq = -1 ;
		}
	} else if ((i % 2 == 0) && (v >= vmin)) {
		// Negative v (case 1)
		if ((*(V + 30 + v) > 0)				&&
		    (*(V + 30 + v + 1) > 0)			&&
		    (*(V + 30 + v + 2) > 0)			&&
		    (*(V + 30 + v + 1) > *(V + 30 + v))		&&
		    (*(V + 30 + v + 1) > *(V + 30 + v + 2))	&&
		    (*(V + 30 + v + 1) > SIGMIN)) {

			swp_freq = fOSC * 1000 + afcex_freq + swp_ofs;
			*SIG1 = *(V + 30 + v + 1);
		} else if ((v + 1 == vmax)			&&
			   (*(V + 30 + v) >= 0)			&&
			   (*(V + 30 + v + 1) >= 0)		&&
			   (*(V + 30 + v + 1) > *(V + 30 + v))	&&
			   (*(V + 30 + v + 1) > SIGMIN)) {
			// (case 2)
			swp_freq = fOSC * 1000 + afcex_freq + swp_ofs;
			*SIG1 = *(V + 30 + v);
		} else if ((v == vmin)				&&
			   (*(V + 30 + v) > 0)			&&
			   (*(V + 30 + v + 1) > 0)		&&
			   (*(V + 30 + v + 2) > 0)		&&
			   (*(V + 30 + v) > *(V + 30 + v + 1))	&&
			   (*(V + 30 + v) > *(V + 30 + v + 2))	&&
			   (*(V + 30 + v) > SIGMIN)) {
			// (case 3)
			swp_freq = fOSC * 1000 + afcex_freq;
			*SIG1 = *(V + 30 + v);
		} else if ((*(V + 30 + v) >= 0)			&&
			   (*(V + 30 + v + 1) >= 0)		&&
			   (*(V + 30 + v + 2) >= 0)		&&
			   (*(V +30 + v + 3) >= 0)		&&
			   (*(V + 30 + v + 1) > *(V + 30 + v))	&&
			   (*(V + 30 + v + 2) > *(V + 30 + v + 3)) &&
			   ((*(V + 30 + v + 1) > SIGMIN)	||
			    (*(V + 30 + v + 2) > SIGMIN))) {
			// (case 4)
			if (*(V + 30 + v + 1) >= *(V + 30 + v + 2)) {
				swp_freq = fOSC * 1000 + afcex_freq + swp_ofs;
				*SIG1 = *(V + 30 + v + 1);
			} else {
				swp_freq = fOSC * 1000 + afcex_freq + swp_ofs * 2;
				*SIG1 = *(V + 30 + v + 2);
			}
		} else if ((*(V + 30 + v) >= 0)			&&
			   (*(V + 30 + v + 1) >= 0)		&&
			   (*(V + 30 + v + 2) >= 0)		&&
			   (*(V + 30 + v + 3) >= 0)		&&
			   (*(V + 30 + v) > *(V + 30 + v + 2))	&&
			   (*(V + 30 + v + 1) > *(V + 30 + v + 2)) &&
			   (*(V + 30 + v) > *(V + 30 + v + 3))	&&
			   (*(V + 30 + v + 1) > *(V + 30 + v + 3)) &&
			   ((*(V + 30 + v) > SIGMIN)		||
			    (*(V + 30 + v + 1) > SIGMIN))) {
			// (case 5)
			if (*(V + 30 + v) >= *(V + 30 + v + 1)) {
				swp_freq = fOSC * 1000 + afcex_freq;
				*SIG1 = *(V + 30 + v);
			} else {
				swp_freq = fOSC * 1000 + afcex_freq + swp_ofs;
				*SIG1 = *(V + 30 + v + 1);
			}
		} else if ((v + 2 == vmin)			&&
			   (*(V + 30 + v) >= 0)			&&
			   (*(V + 30 + v + 1) >= 0)		&&
			   (*(V + 30 + v + 2) >= 0)		&&
			   (*(V + 30 + v + 1) > *(V + 30 + v))	&&
			   (*(V + 30 + v + 2) > *(V + 30 + v))	&&
			   ((*(V + 30 + v + 1) > SIGMIN)	||
			    (*(V + 30 + v + 2) > SIGMIN))) {
			// (case 6)
			if (*(V + 30 + v + 1) >= *(V + 30 + v + 2)) {
				swp_freq = fOSC * 1000 + afcex_freq + swp_ofs;
				*SIG1 = *(V + 30 + v + 1);
			} else {
				swp_freq = fOSC * 1000 + afcex_freq + swp_ofs * 2;
				*SIG1 = *(V + 30 + v + 2);
			}
		} else if ((vmax == 0) && (vmin == 0) && (*(V + 30 + v) > SIGMIN)) {
			swp_freq = fOSC * 1000;
			*SIG1 = *(V + 30 + v);
		} else swp_freq = -1;
	} else swp_freq = -1;

	return swp_freq;
}


static void swp_info_get2(struct mb86a16_state *state,
			  int smrt,
			  int R,
			  int swp_freq,
			  int clkmst,
			  int *afcex_freq,
			  int *fOSC,
			  unsigned char *AFCEX_L,
			  unsigned char *AFCEX_H)
{
	int AFCEX ;

	if (R == 0)
		*fOSC = (swp_freq + 1000) / 2000 * 2;
	else
		*fOSC = (swp_freq + 500) / 1000;

	if (*fOSC >= swp_freq)
		*afcex_freq = *fOSC * 1000 - swp_freq;
	else
		*afcex_freq = swp_freq - *fOSC * 1000;

	AFCEX = *afcex_freq * 8192 / clkmst;
	*AFCEX_L =  AFCEX & 0x00ff;
	*AFCEX_H = (AFCEX & 0x0f00) >> 8;
}

static void afcex_info_get(struct mb86a16_state *state,
			   int clkmst,
			   int afcex_freq,
			   unsigned char *AFCEX_L,
			   unsigned char *AFCEX_H)
{
	int AFCEX ;

	AFCEX = afcex_freq * 8192 / clkmst;
	*AFCEX_L =  AFCEX & 0x00ff;
	*AFCEX_H = (AFCEX & 0x0f00) >> 8;
}

static int SEQ_set(struct mb86a16_state *state, unsigned char loop)
{
	// SLOCK0 = 0
	if (mb86a16_write(state, 0x32, 0x02 | (loop << 2)) != 1) {
		dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
		return -EREMOTEIO;
	}

	return 0;
}

static int iq_vt_set(struct mb86a16_state *state, unsigned char IQINV)
{
	// Viterbi Rate, IQ Settings
	if (mb86a16_write(state, 0x06, 0xdf | (IQINV << 5)) != 1) {
		dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
		return -EREMOTEIO;
	}

	return 0;
}

static int FEC_srst(struct mb86a16_state *state)
{
	if (mb86a16_write(state, 0x0c, 0x02) != 1) {
		dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
		return -EREMOTEIO;
	}

	return 0;
}

static int S2T_set(struct mb86a16_state *state, unsigned char S2T)
{
	if (mb86a16_write(state, 0x34, 0x70 | S2T) != 1) {
		dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
		return -EREMOTEIO;
	}

	return 0;
}

static int S45T_set(struct mb86a16_state *state, unsigned char S4T, unsigned char S5T)
{
	if (mb86a16_write(state, 0x35, 0x00 | (S5T << 4) | S4T) != 1) {
		dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
		return -EREMOTEIO;
	}

	return 0;
}


static int mb86a16_set_fe(struct mb86a16_state *state,
			  u32 freq,
			  u32 symb)
{
	u8 agcval, cnmval;

	int fTP, smrt;
	int i, j;
	int fOSC = 0;
	int fOSC_start = 0;
	int clkmst;	//internal clock freq[kHz]
	int wait_t;
	int fcp;
	int swp_ofs;
	int V[60];
	u8 SIG1MIN;

	unsigned char DECI, CSEL, RSEL;
	unsigned char CREN, AFCEN, AFCEXEN;
	unsigned char SIG1;
	unsigned char TIMINT1, TIMINT2, TIMEXT;
	unsigned char S0T, S1T;
	unsigned char S2T;
	unsigned char S4T, S5T;
	unsigned char AFCEX_L, AFCEX_H;
	unsigned char R;
	unsigned char VIRM;
	unsigned char ETH, VIA;
	unsigned char junk;

	int loop;
	int ftemp;
	int v, vmax, vmin;
	int vmax_his, vmin_his;
	int swp_freq, prev_swp_freq[20];
	int prev_freq_num;
	int signal_dupl;
	int afcex_freq;
	int signal;
	int afcerr;
	int temp_freq, delta_freq;
	int dagcm[4];
	int smrt_d;
	int ret = -1;
	int sync;

	fTP = (int) freq / 1000;
	smrt = (int) symb / 1000;

	dprintk(verbose, MB86A16_DEBUG, 1, "freq=%d Mhz, symbrt=%d Ksps", fTP, smrt);

	fcp = 3000;     //capture range of carrier recovery[kHz]
	swp_ofs = smrt / 4;

	for (i = 0; i < 60; i++)
		V[i] = -1;

	for (i = 0; i < 20; i++)
		prev_swp_freq[i] = 0;

	SIG1MIN = 25;


	SEQ_set(state, 0);
	iq_vt_set(state, 0);

	CREN = 0;
	AFCEN = 0;
	AFCEXEN = 1;
	TIMINT1 = 0;
	TIMINT2 = 1;
	TIMEXT = 2;
	S1T = 0;
	S0T = 0;

	if (initial_set(state) < 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "initial set failed");
		return -1;
	}
	if (DAGC_data_set(state, 3, 2) < 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "DAGC data set error");
		return -1;
	}
	if (EN_set(state, CREN, AFCEN) < 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "EN set error");
		return -1; // (0, 0)
	}
	if (AFCEXEN_set(state, AFCEXEN, smrt) < 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "AFCEXEN set error");
		return -1; // (1, smrt) = (1, symbolrate)
	}
	if (CNTM_set(state, TIMINT1, TIMINT2, TIMEXT) < 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "CNTM set error");
		return -1; // (0, 1, 2)
	}
	if (S01T_set(state, S1T, S0T) < 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "S01T set error");
		return -1; // (0, 0)
	}
	smrt_info_get(state, smrt, &DECI, &CSEL, &RSEL, &clkmst);
	if (smrt_set(state, smrt, DECI, CSEL, RSEL, clkmst) < 0) {
		dprintk(verbose, MB86A16_ERROR, 1, "smrt info get error");
		return -1;
	}

	R = vco_dev_get(state, smrt);
	if (R == 1)
		fOSC_start = fTP;
	else if (R == 0) {
		if (fTP % 2 == 0) {
			fOSC_start = fTP;
		} else {
			fOSC_start = fTP + 1;
			if (fOSC_start > 2150)
				fOSC_start = fTP - 1;
		}
	}
	loop = 1;
	ftemp = fOSC_start * 1000;
	vmax = 0 ;
	while (loop == 1) {
		ftemp = ftemp + swp_ofs;
		vmax++;

		// Upper bound
		if (ftemp > 2150000) {
			loop = 0;
			vmax--;
		}
		else if ((ftemp == 2150000) || (ftemp - fTP * 1000 >= fcp + smrt / 4))
			loop = 0;
	}

	loop = 1;
	ftemp = fOSC_start * 1000;
	vmin = 0 ;
	while (loop == 1) {
		ftemp = ftemp - swp_ofs;
		vmin--;

		// Lower bound
		if (ftemp < 950000) {
			loop = 0;
			vmin++;
		}
		else if ((ftemp == 950000) || (fTP * 1000 - ftemp >= fcp + smrt / 4))
			loop = 0;
	}

	wait_t = (8000 + smrt / 2) / smrt;
	if (wait_t == 0)
		wait_t = 1;

	i = 0;
	j = 0;
	prev_freq_num = 0;
	loop = 1;
	signal = 0;
	vmax_his = 0;
	vmin_his = 0;
	v = 0;

	while (loop == 1) {
		swp_info_get(state, fOSC_start, smrt, clkmst,
				 v, R, swp_ofs, &fOSC,
				 &afcex_freq, &AFCEX_L, &AFCEX_H);

		if (rf_val_set(state, fOSC, smrt,R) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "rf val set error");
			return -1;
		}

		if (afcex_data_set(state, AFCEX_L, AFCEX_H) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "afcex data set error");
			return -1;
		}
		if (srst(state) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "srst error");
			return -1;
		}
		msleep_interruptible(wait_t);

		if (mb86a16_read(state, 0x37, &SIG1) != 2) {
			dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
			return -1;
		}
		V[30 + v] = SIG1 ;
		swp_freq = swp_freq_calcuation(state, i, v, V, vmax, vmin,
						SIG1MIN, fOSC, afcex_freq,
						swp_ofs, &SIG1);	//changed

		signal_dupl = 0;
		for (j = 0; j < prev_freq_num; j++) {
			if ((ABS(prev_swp_freq[j] - swp_freq)) < (swp_ofs * 3 / 2)) {
				signal_dupl = 1;
				dprintk(verbose, MB86A16_INFO, 1, "Probably Duplicate Signal, j = %d", j);
			}
		}
		if ((signal_dupl == 0) && (swp_freq > 0) && (ABS(swp_freq - fTP * 1000) < fcp + smrt / 6)) {
			dprintk(verbose, MB86A16_DEBUG, 1, "------ Signal detect ------ [swp_freq=[%07d, srate=%05d]]", swp_freq, smrt);
			prev_swp_freq[prev_freq_num] = swp_freq;
			prev_freq_num++;
			swp_info_get2(state, smrt, R, swp_freq, clkmst,
					&afcex_freq, &fOSC,
					&AFCEX_L, &AFCEX_H);

			if (rf_val_set(state, fOSC, smrt, R) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "rf val set error");
				return -1;
			}
			if (afcex_data_set(state, AFCEX_L, AFCEX_H) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "afcex data set error");
				return -1;
			}
			signal = signal_det(state, smrt, &DECI, &CSEL, &RSEL, &clkmst, &SIG1);
			if (signal == 1) {
				dprintk(verbose, MB86A16_DEBUG, 1, "***** Signal Found *****");
				loop = 0;
			} else {
				dprintk(verbose, MB86A16_ERROR, 1, "!!!!! No signal !!!!!, try again...");
				smrt_info_get(state, smrt, &DECI, &CSEL, &RSEL, &clkmst);
				if (smrt_set(state, smrt, DECI, CSEL, RSEL, clkmst) < 0) {
					dprintk(verbose, MB86A16_ERROR, 1, "smrt set error");
					return -1;
				}
			}
		}
		if (v > vmax)
			vmax_his = 1 ;
		if (v < vmin)
			vmin_his = 1 ;
		i++;

		if ((i % 2 == 1) && (vmax_his == 1))
			i++;
		if ((i % 2 == 0) && (vmin_his == 1))
			i++;

		if (i % 2 == 1)
			v = (i + 1) / 2;
		else
			v = -i / 2;

		if ((vmax_his == 1) && (vmin_his == 1))
			loop = 0 ;
	}

	if (signal == 1) {
		S1T = 7 ;
		S0T = 1 ;
		CREN = 0 ;
		AFCEN = 1 ;
		AFCEXEN = 0 ;

		if (S01T_set(state, S1T, S0T) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "S01T set error");
			return -1;
		}
		smrt_info_get(state, smrt, &DECI, &CSEL, &RSEL, &clkmst);
		if (smrt_set(state, smrt, DECI, CSEL, RSEL, clkmst) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "smrt set error");
			return -1;
		}
		if (EN_set(state, CREN, AFCEN) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "EN set error");
			return -1;
		}
		if (AFCEXEN_set(state, AFCEXEN, smrt) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "AFCEXEN set error");
			return -1;
		}
		afcex_info_get(state, clkmst, afcex_freq, &AFCEX_L, &AFCEX_H);
		if (afcofs_data_set(state, AFCEX_L, AFCEX_H) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "AFCOFS data set error");
			return -1;
		}
		if (srst(state) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "srst error");
			return -1;
		}
		// delay 4~200
		wait_t = 200000 / clkmst + 200000 / smrt;
		msleep(wait_t);
		afcerr = afcerr_chk(state, clkmst);
		if (afcerr == -1)
			return -1;

		swp_freq = fOSC * 1000 + afcerr ;
		AFCEXEN = 1 ;
		if (smrt >= 1500)
			smrt_d = smrt / 3;
		else
			smrt_d = smrt / 2;
		smrt_info_get(state, smrt_d, &DECI, &CSEL, &RSEL, &clkmst);
		if (smrt_set(state, smrt_d, DECI, CSEL, RSEL, clkmst) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "smrt set error");
			return -1;
		}
		if (AFCEXEN_set(state, AFCEXEN, smrt_d) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "AFCEXEN set error");
			return -1;
		}
		R = vco_dev_get(state, smrt_d);
		if (DAGC_data_set(state, 2, 0) < 0) {
			dprintk(verbose, MB86A16_ERROR, 1, "DAGC data set error");
			return -1;
		}
		for (i = 0; i < 3; i++) {
			temp_freq = swp_freq + (i - 1) * smrt / 8;
			swp_info_get2(state, smrt_d, R, temp_freq, clkmst, &afcex_freq, &fOSC, &AFCEX_L, &AFCEX_H);
			if (rf_val_set(state, fOSC, smrt_d, R) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "rf val set error");
				return -1;
			}
			if (afcex_data_set(state, AFCEX_L, AFCEX_H) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "afcex data set error");
				return -1;
			}
			wait_t = 200000 / clkmst + 40000 / smrt_d;
			msleep(wait_t);
			dagcm[i] = dagcm_val_get(state);
		}
		if ((dagcm[0] > dagcm[1]) &&
			(dagcm[0] > dagcm[2]) &&
			(dagcm[0] - dagcm[1] > 2 * (dagcm[2] - dagcm[1]))) {

			temp_freq = swp_freq - 2 * smrt / 8;
			swp_info_get2(state, smrt_d, R, temp_freq, clkmst, &afcex_freq, &fOSC, &AFCEX_L, &AFCEX_H);
			if (rf_val_set(state, fOSC, smrt_d, R) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "rf val set error");
				return -1;
			}
			if (afcex_data_set(state, AFCEX_L, AFCEX_H) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "afcex data set");
				return -1;
			}
			wait_t = 200000 / clkmst + 40000 / smrt_d;
			msleep(wait_t);
			dagcm[3] = dagcm_val_get(state);
			if (dagcm[3] > dagcm[1])
				delta_freq = (dagcm[2] - dagcm[0] + dagcm[1] - dagcm[3]) * smrt / 300;
			else
				delta_freq = 0;
		} else if ((dagcm[2] > dagcm[1]) &&
				(dagcm[2] > dagcm[0]) &&
				(dagcm[2] - dagcm[1] > 2 * (dagcm[0] - dagcm[1]))) {

			temp_freq = swp_freq + 2 * smrt / 8;
			swp_info_get2(state, smrt_d, R, temp_freq, clkmst, &afcex_freq, &fOSC, &AFCEX_L, &AFCEX_H);
			if (rf_val_set(state, fOSC, smrt_d, R) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "rf val set");
				return -1;
			}
			if (afcex_data_set(state, AFCEX_L, AFCEX_H) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "afcex data set");
				return -1;
			}
			wait_t = 200000 / clkmst + 40000 / smrt_d;
			msleep(wait_t);
			dagcm[3] = dagcm_val_get(state);
			if (dagcm[3] > dagcm[1])
				delta_freq = (dagcm[2] - dagcm[0] + dagcm[3] - dagcm[1]) * smrt / 300;
			else
				delta_freq = 0 ;

		} else {
			delta_freq = 0 ;
		}		
		swp_freq += delta_freq;		
		if (ABS(fTP * 1000 - swp_freq) > 3800) {
			dprintk(verbose, MB86A16_ERROR, 1, "NO  --  SIGNAL !");
		} else {

			S1T = 0;
			S0T = 3;
			CREN = 1;
			AFCEN = 0;
			AFCEXEN = 1;

			if (S01T_set(state, S1T, S0T) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "S01T set error");
				return -1;
			}
			if (DAGC_data_set(state, 0, 0) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "DAGC data set error");
				return -1;
			}
			R = vco_dev_get(state, smrt);
			smrt_info_get(state, smrt, &DECI, &CSEL, &RSEL, &clkmst);
			if (smrt_set(state, smrt, DECI, CSEL, RSEL, clkmst) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "smrt set error");
				return -1;
			}
			if (EN_set(state, CREN, AFCEN) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "EN set error");
				return -1;
			}
			if (AFCEXEN_set(state, AFCEXEN, smrt) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "AFCEXEN set error");
				return -1;
			}
			swp_info_get2(state, smrt, R, swp_freq, clkmst, &afcex_freq, &fOSC, &AFCEX_L, &AFCEX_H);
			if (rf_val_set(state, fOSC, smrt, R) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "rf val set error");
				return -1;
			}
			if (afcex_data_set(state, AFCEX_L, AFCEX_H) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "afcex data set error");
				return -1;
			}
			if (srst(state) < 0) {
				dprintk(verbose, MB86A16_ERROR, 1, "srst error");
				return -1;
			}
			wait_t = 7 + (10000 + smrt / 2) / smrt;
			if (wait_t == 0)
				wait_t = 1;
			msleep_interruptible(wait_t);
			if (mb86a16_read(state, 0x37, &SIG1) != 2) {
				dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
				return -EREMOTEIO;
			}

			if (SIG1 > 110) {
				S2T = 4; S4T = 1; S5T = 6; ETH = 4; VIA = 6;
				wait_t = 7 + (917504 + smrt / 2) / smrt;
			} else if (SIG1 > 105) {
				S2T = 4; S4T = 2; S5T = 8; ETH = 7; VIA = 2;
				wait_t = 7 + (1048576 + smrt / 2) / smrt;
			} else if (SIG1 > 85) {
				S2T = 5; S4T = 2; S5T = 8; ETH = 7; VIA = 2;
				wait_t = 7 + (1310720 + smrt / 2) / smrt;
			} else if (SIG1 > 65) {
				S2T = 6; S4T = 2; S5T = 8; ETH = 7; VIA = 2;
				wait_t = 7 + (1572864 + smrt / 2) / smrt;
			} else {
				S2T = 7; S4T = 2; S5T = 8; ETH = 7; VIA = 2;
				wait_t = 7 + (2097152 + smrt / 2) / smrt;
			}
			S2T_set(state, S2T);
			S45T_set(state, S4T, S5T);
			Vi_set(state, ETH, VIA);
			srst(state);
			msleep_interruptible(wait_t);
			sync = sync_chk(state, &VIRM);
			dprintk(verbose, MB86A16_DEBUG, 1, "-------- Viterbi=[%d] SYNC=[%d] ---------", VIRM, sync);
			if (mb86a16_read(state, 0x0d, &state->signal) != 2) {
				dprintk(verbose, MB86A16_ERROR, 1, "I2C transfer error");
				return -EREMOTEIO;
			}
			if (VIRM) {
				if (VIRM == 4) { // 5/6
					if (SIG1 > 110)
						wait_t = ( 786432 + smrt/2) / smrt;
					else
						wait_t = (1572864 + smrt/2) / smrt;
					if (smrt < 5000)
						// FIXME ! , should be a long wait !
						msleep_interruptible(wait_t);
					else
						msleep_interruptible(wait_t);

					if (sync_chk(state, &junk) == 0) {
						iq_vt_set(state, 1);
						FEC_srst(state);
					}
					if (SIG1 > 110)
						wait_t = ( 786432 + smrt/2) / smrt;
					else
						wait_t = (1572864 + smrt/2) / smrt;

					msleep_interruptible(wait_t);
					SEQ_set(state, 1);
				} else { // 1/2, 2/3, 3/4, 7/8
					if (SIG1 > 110)
						wait_t = ( 786432 + smrt/2) / smrt;
					else
						wait_t = (1572864 + smrt/2) / smrt;

					msleep_interruptible(wait_t);
					SEQ_set(state, 1);
				}
			} else {
				dprintk(verbose, MB86A16_ERROR, 1, "NO  -- SIGNAL");
				SEQ_set(state, 1);
			}
		}
	} else {
		dprintk (verbose, MB86A16_ERROR, 1, "NO  -- SIGNAL");
	}

	sync = sync_chk(state, &junk);
	if (sync) {
		dprintk(verbose, MB86A16_ERROR, 1, "******* SYNC *******");
		freqerr_chk(state, fTP, smrt, 1);
	}
	mb86a16_read(state, 0x15, &agcval);
	mb86a16_read(state, 0x26, &cnmval);
	dprintk(verbose, MB86A16_DEBUG, 1, "AGC = %02x CNM = %02x", agcval, cnmval);

	return ret;
}

#define MB86A16_FE_ALGO		1

static int mb86a16_frontend_algo(struct dvb_frontend *fe)
{
	return MB86A16_FE_ALGO;
}

static int mb86a16_set_frontend(struct dvb_frontend *fe,
				struct dvb_frontend_parameters *p,
				unsigned int mode_flags,
				int *delay,
				fe_status_t *status)
{
	int ret = 0;
	struct mb86a16_state *state = fe->demodulator_priv;

	TRACE_IN;
	//dprintk(verbose, MB86A16_ERROR, 1, "freq = %d symb = %d", p->frequency, p->u.qpsk.symbol_rate);
	if (p != NULL)
		ret = mb86a16_set_fe(state, p->frequency, p->u.qpsk.symbol_rate);

	if (!(mode_flags & FE_TUNE_MODE_ONESHOT))
		mb86a16_read_status(fe, status);

	*delay = HZ/3000;

	TRACE_OUT;
		dprintk(verbose, MB86A16_DEBUG, 1, "mb86a16_set_frontend end");
	return ret;
}

static void mb86a16_release(struct dvb_frontend *fe)
{
	struct mb86a16_state *state = fe->demodulator_priv;
	kfree(state);
}

static int mb86a16_init(struct dvb_frontend *fe)
{
	dprintk(verbose, MB86A16_DEBUG, 1, "mb86a16_init");
	return 0;
}

static int mb86a16_sleep(struct dvb_frontend *fe)
{
	return 0;
}

static int mb86a16_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct mb86a16_state* state = fe->demodulator_priv;
	u8 tmp;
	s32 signal;

	mb86a16_read(state, 0x15, &tmp);
	signal=  0xffff - tmp;
	signal = signal * 5 / 8;
	*strength = (signal > 0xffff) ? 0xffff : (signal < 0) ? 0 : signal;

	return 0;
}

static int mb86a16_read_snr(struct dvb_frontend *fe, u16 *snr)
{
	struct mb86a16_state* state = fe->demodulator_priv;
	u8 tmp;
	s32 xsnr;

	mb86a16_read(state, 0x26, &tmp);
	xsnr = 0xff00 + tmp;
	*snr = xsnr;

	return 0;
}

static struct dvb_frontend_ops mb86a16_ops = {
	.info = {
		.name			= "Fujitsu MB86A16 DVB-S",
		.type			= FE_QPSK,
		.frequency_min		= 950000,
		.frequency_max		= 2150000,
		.frequency_stepsize	= 125,
		.frequency_tolerance	= 0,
		.symbol_rate_min	= 1000000,
		.symbol_rate_max	= 45000000,
		.symbol_rate_tolerance	= 500,
		.caps			= FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 |
					  FE_CAN_FEC_3_4 | FE_CAN_FEC_5_6 |
					  FE_CAN_FEC_7_8 | FE_CAN_QPSK    |
					  FE_CAN_FEC_AUTO
	},
	.release			= mb86a16_release,
	.tune				= mb86a16_set_frontend,	
	.get_frontend_algo		= mb86a16_frontend_algo,
	.init				= mb86a16_init,
	.sleep				= mb86a16_sleep,

	.diseqc_send_master_cmd 	= mb86a16_send_diseqc_msg,	
	.diseqc_send_burst 		= mb86a16_send_diseqc_burst,	
	.set_tone 			= mb86a16_send_tone_msg,	

	.read_status			= mb86a16_read_status,	
	.read_signal_strength		= mb86a16_read_signal_strength,
	.read_snr			= mb86a16_read_snr,
};

struct dvb_frontend *mb86a16_attach(const struct mb86a16_config *config,
				    struct i2c_adapter *i2c_adap)
{
	u8 dev_id = 0;
	struct mb86a16_state *state = NULL;

	state = kmalloc(sizeof (struct mb86a16_state), GFP_KERNEL);
	if (state == NULL)
		goto error;

	state->config = config;
	state->i2c_adap = i2c_adap;

	mb86a16_read(state, 0x7f, &dev_id);
	if (dev_id != 0xfe)
		goto error;

	memcpy(&state->frontend.ops, &mb86a16_ops, sizeof (struct dvb_frontend_ops));
	state->frontend.demodulator_priv = state;
	state->frontend.ops.set_voltage = state->config->set_voltage;

	return &state->frontend;
error:
	kfree(state);
	return NULL;
}

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(mb86a16_attach);

