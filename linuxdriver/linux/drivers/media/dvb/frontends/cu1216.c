/*
	CU-1216 driver for the Mantis bridge based cards

	Copyright (C) 2005 Twinhan Technology Co. Ltd
		based on the TDA 10021 driver

	Copyright (C) 1999 Convergence Integrated Media GmbH <ralph@convergence.de>
	Copyright (C) 2004 Markus Schulz <msc@antzsystem.de>

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

//#include <linux/config.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>

#include "dvb_frontend.h"
#include "mantis_core.h"
#include "cu1216.h"
#include "cu1216_regs.h"

unsigned int verbose = 1;
unsigned char type; //TDA10021 or TDA10023
module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "print AFC offset after tuning for debugging the PWM setting");


typedef struct AC_TypeQAM_TAG {
	u8 bConf;
	u8 bAgcref;
	u8 bLockthr;
	u8 bMseth;
	u8 bAref;
} AC_TypeQAM_T;

typedef struct TDA10023_QAM_TAG{
	u8  uQam;
    	u8  uLockthr;
    	u8  uMseth;
    	u8  uAref;
    	u8  uAgcRefNyq;
    	u8  uErAgcNyqThd;
} TDA10023Qam_t;

static void  cu1216_set_symbolRate(struct dvb_frontend *fe, u16 uFreqSymb);
static void  cu1216_set_QAM       (struct dvb_frontend *fe, u8  bQAM);
static void  cu1216_set_IQ        (struct dvb_frontend *fe, u8  bSI);
static void  cu1216_set_gain      (struct dvb_frontend *fe, u8  bGain);
static void  cu1216_clear_register(struct dvb_frontend *fe);
static int   cu1216_init          (struct dvb_frontend *fe);
static int   tda10023_set_tuner(struct dvb_frontend *fe, struct dvb_frontend_parameters *params);
static int tda10023_writereg (struct cu1216_state *state, u8 reg, u8 data);


static int tda10023_writereg (struct cu1216_state *state, u8 reg, u8 data)
{
	u8 buf[] = { reg, data };
	int ret;

	struct i2c_msg msg = {
		.addr = 0xc0 << 1,
		.flags = 0,
		.buf = buf,
		.len = 2
	};	

	ret = i2c_transfer (state->i2c, &msg, 1);
	if (ret != 1)
		printk("DVB: TDA10023(%d): %s, writereg error ""(reg == 0x%02x, val == 0x%02x, ret == %i)\n", state->frontend.dvb->num, __FUNCTION__, reg, data, ret);

	msleep(10);
	return (ret != 1) ? -EREMOTEIO : 0;
}

static int cu1216_writereg (struct cu1216_state *state, u8 reg, u8 data)
{
	u8 buf[] = { reg, data };

	struct i2c_msg msg = {
		.addr = state->config->demod_address,
		.flags = 0,
		.buf = buf,
		.len = 2
	};
	int ret;

	ret = i2c_transfer (state->i2c, &msg, 1);
	if (ret != 1)
		printk("DVB: TDA10021(%d): %s, writereg error ""(reg == 0x%02x, val == 0x%02x, ret == %i)\n", state->frontend.dvb->num, __FUNCTION__, reg, data, ret);

	msleep(10);
	return (ret != 1) ? -EREMOTEIO : 0;
}

static u8 cu1216_readreg (struct cu1216_state *state, u8 reg)
{
	u8 b0 [] = { reg };
	u8 b1 [] = { 0 };
	struct i2c_msg msg [] = {
		{
			.addr = state->config->demod_address,
			.flags = 0,
			.buf = b0,
			.len = 1
		},
		{
			.addr = state->config->demod_address,
			.flags = I2C_M_RD,
			.buf = b1,
			.len = 1
		}
	};
	int ret;

	ret = i2c_transfer (state->i2c, msg, 2);
	if (ret != 2)
		printk("DVB: TDA10021: %s: readreg error (ret == %i)\n",__FUNCTION__, ret);
	return b1[0];
}

static int cu1216_writereg_mask (struct cu1216_state *state, u8 reg, u8 mask, u8 data)
{
	u8  value;

	value = cu1216_readreg(state, reg);
	value = (value & ~mask) | (data & mask);
	
	return cu1216_writereg(state, reg, value);
}


//get access to tuner
static int lock_tuner(struct cu1216_state *state)
{
	u8 buf[2] = { 0x0f, 0x40 | 0x80 };
	struct i2c_msg msg = {
		.addr = state->config->demod_address,
		.flags = 0,
		.buf = buf,
		.len = 2
	};

	if (i2c_transfer(state->i2c, &msg, 1) != 1) {
		printk("cu1216: lock tuner fails\n");
		return -EREMOTEIO;
	}
	return 0;
}

//release access from tuner
static int unlock_tuner(struct cu1216_state *state)
{
	u8 buf[2] = { 0x0f, 0x40 & 0x7f };
	struct i2c_msg msg_post = {
		.addr = state->config->demod_address,
		.flags = 0,
		.buf = buf,
		.len = 2
	};

	if (i2c_transfer(state->i2c, &msg_post, 1) != 1) {
		printk("cu1216: unlock tuner fails\n");
		return -EREMOTEIO;
	}

	return 0;
}

static void cu1216_set_symbolRate(struct dvb_frontend *fe, u16 uFreqSymb)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8    pWrite[4], bNdec, bSFil;
	u32  uBDR, uBDRb ,uFreqSymbInv, uFreqSymb480;
        u32  uSR,uSysClk;
	u32  fSysClk;	

        if(uFreqSymb == 0)
		return ;	

	if(type == MK1)
	{
		// calculate the system frequency
		fSysClk  = OM5734_XTALFREQ_DEF * (OM5734_PLLMFACTOR_DEF + 1);
		fSysClk /= (OM5734_PLLNFACTOR_DEF + 1) * (OM5734_PLLPFACTOR_DEF + 1);

		// add 480 ppm to the SR
		uFreqSymb480  = uFreqSymb;
		uFreqSymb480  = uFreqSymb * 480;      // I Don't Know why it * 480 not + 480
		uFreqSymb480 /= 1000000L;
		uFreqSymb480  =	uFreqSymb + uFreqSymb480;
		fSysClk = fSysClk/1000;
		bNdec = 0;
		bSFil = 1;

		if (((fSysClk / 123) < (uFreqSymb480 / 10)) && ((uFreqSymb480 / 10)  <= (fSysClk / 80)))
			bSFil = 0;

		if (((fSysClk / 246) < (uFreqSymb480 / 10)) && ((uFreqSymb480 / 10)  <= (fSysClk / 160)))
			bSFil = 0;

		if (((fSysClk / 492) < (uFreqSymb480 / 10)) && ((uFreqSymb480 / 10)  <= (fSysClk / 320)))
			bSFil = 0;

		if (((fSysClk / 984) < (uFreqSymb480 / 10)) && ((uFreqSymb480 / 10)  <= (fSysClk / 640)))
			bSFil = 0;

		// program SFIL
		cu1216_writereg(state, AC_GAIN_IND, 0x23);

		// program NDEC
		cu1216_writereg(state, AC_CLKCONF_IND, 0x0a);

		//---------------------------------------
		// program the symbol frequency registers
		//---------------------------------------
		// calculate the inversion of the symbol frequency
		uFreqSymbInv   = fSysClk * 16;      // prefer to P21/58 Ice_Deng 2003/12/20
		uFreqSymbInv >>= bNdec;		    // divide by 2^decim
		uFreqSymbInv  += uFreqSymb / 2;	    // rounding for division
		uFreqSymbInv  /= uFreqSymb;

		if (uFreqSymbInv > 255)
			uFreqSymbInv = 255;

		uBDRb = 1;
		uBDRb = uBDRb << (24 + bNdec);

		fSysClk = fSysClk / 10;
		uBDR = uBDRb / fSysClk;
		uBDR *= uFreqSymb;

		uBDRb %= fSysClk;
		uBDRb *= uFreqSymb;
		uBDRb /= fSysClk;
		uBDR  += uBDRb;
		uBDR  /= 10;

		// program the value in register of the symbol rate
		pWrite[0] = (unsigned char)(uBDR);
		pWrite[1] = (unsigned char)(uBDR >> 8);
		pWrite[2] = (unsigned char)(uBDR >> 16);
		pWrite[3] = (unsigned char)uFreqSymbInv;

		cu1216_writereg(state, AC_BDRLSB_IND, pWrite[0]);
		cu1216_writereg(state, AC_BDRMID_IND, pWrite[1]);
		cu1216_writereg(state, AC_BDRMSB_IND, pWrite[2]);
		cu1216_writereg(state, AC_BDRINV_IND, pWrite[3]);
	}
	else
	{	
		uSR = uFreqSymb;
		// calculate the system frequency
		uSysClk = CU1216_XTALL_FREQ_28 * (CU1216_PLLMFACTOR_DVB_DEF+1);
		uSysClk /= (CU1216_PLLNFACTOR_DVB_DEF+1)*(CU1216_PLLPFACTOR_DVB_DEF+1);
		uSysClk /=1000;
	
		if (uSR/10 < uSysClk/984)
		{
			bNdec = 3;
			bSFil = 1;
		}
		else if (uSR/10 < uSysClk/640)
		{
			bNdec = 3;
			bSFil = 0;
		}
		else if (uSR/10 < uSysClk/492)
		{
			bNdec = 2;
			bSFil = 1;
		}
		else if (uSR/10 < uSysClk/320)
		{
			bNdec = 2;
			bSFil = 0;
		}
		else if (uSR/10 < uSysClk/246)
		{
			bNdec = 1;
			bSFil = 1;
		}
		else if (uSR/10 < uSysClk/160)
		{
			bNdec = 1;
			bSFil = 0;
		}
		else if (uSR/10 < uSysClk/123)
		{
			bNdec = 0;
			bSFil = 1;
		}
		else
		{
			bNdec = 0;
			bSFil = 0;
		}		 
		// program SFIL
		cu1216_writereg_mask( state, 0x3d, 0x80, bSFil << 7);

		// program NDEC
		cu1216_writereg_mask( state, 0x03, 0xc0, bNdec << 6);

		//---------------------------------------
		// program the symbol frequency registers
		//---------------------------------------
		// calculate the inversion of the symbol frequency
		uFreqSymbInv	 = uSysClk*16;
		uFreqSymbInv >>= bNdec;
		uFreqSymbInv	+= uFreqSymb/2;
		uFreqSymbInv	/= uFreqSymb;
		if (uFreqSymbInv > 255)
		{
			uFreqSymbInv = 255;
		}
		// calculate the symbol rate
		uBDRb=1;
		uBDRb  = uBDRb<<(24+bNdec);
		uSysClk=uSysClk/10;
		uBDR   = uBDRb/uSysClk;
		uBDR  *= uFreqSymb;
		
		uBDRb %=uSysClk;
		uBDRb *= uFreqSymb;
		uBDRb /= uSysClk;
		uBDR  +=uBDRb;
		uBDR  /=10;

		// program the value in register of the symbol rate
		pWrite[0] = (u8)(uBDR);
		pWrite[1] = (u8)(uBDR >> 8);
		pWrite[2] = (u8)(uBDR >> 16);
		pWrite[3] = (u8)uFreqSymbInv;

	    	cu1216_writereg(state, AC_BDRLSB_IND, pWrite[0]);
	    	cu1216_writereg(state, AC_BDRMID_IND, pWrite[1]);
	    	cu1216_writereg(state, AC_BDRMSB_IND, pWrite[2]);
	    	cu1216_writereg(state, AC_BDRINV_IND, pWrite[3]);
	}
	
}


static void cu1216_set_QAM(struct dvb_frontend *fe, u8  bQAM)	// The default value is 16 --QAM   
{
	struct cu1216_state *state = fe->demodulator_priv;

	if(type == MK1)
	{
		AC_TypeQAM_T sTypeQAM[] = {
			{ 0x14, 120, 0x78, 114, 0x96 },    // 4 QAM    <=> qam=0
			{ 0x00, 140, 0x6e, 162, 0x91 },    // 16 QAM   <=> qam=1
			{ 0x04, 140, 0x4b, 116, 0x96 },    // 32 QAM   <=> qam=2
			{ 0x08, 106, 0x37, 67, 0x6a  },    // 64 QAM   <=> qam=3
			{ 0x0c, 120, 0x2d, 52, 0x7e  },    // 128 QAM  <=> qam=4
			{ 0x10, 92, 0x23, 35, 0x6b   },    // 256 QAM  <=> qam=5
		};	

		// program the modulation in CONF register
		cu1216_writereg_mask(state, AC_CONF_IND, AC_CONF_QAM_MSK, sTypeQAM[bQAM].bConf);
		// AGCREF
		cu1216_writereg(state, AC_AGCREF_IND, sTypeQAM[bQAM].bAgcref);
		// LOCKTHR
		cu1216_writereg(state, AC_LOCKTHR_IND, sTypeQAM[bQAM].bLockthr);
		// MSETH
		cu1216_writereg(state, AC_MSETH_IND, sTypeQAM[bQAM].bMseth);
		// AREF
		cu1216_writereg(state, AC_AREF_IND, sTypeQAM[bQAM].bAref);
	}
	else
	{
	
 		//  QAM   LOCKTHR  MSETH   AREF AGCREFNYQ  ERAGCNYQ_THD
      		static TDA10023Qam_t sTypeQAM3[] = {
	 		{ 0x14, 0x78, 0x8c, 0x96, 0x78, 0x4c  },// 4 QAM  <=> qam=0
        		{ 0x00, 0x87, 0xa2, 0x91, 0x8c, 0x57  },// 16 QAM <=> qam=1
        		{ 0x04, 0x64, 0x74, 0x96, 0x8c, 0x57  },// 32 QAM <=> qam=2
        		{ 0x08, 0x46, 0x43, 0x6a, 0x6a, 0x44  },// 64 QAM <=> qam=3
        		{ 0x0c, 0x36, 0x34, 0x7e, 0x78, 0x4c  },// 128 QAM<=> qam=4
        		{ 0x10, 0x26, 0x23, 0x6c, 0x5c, 0x3c  } // 256 QAM<=> qam=5
    		};

    		// LOCKTHR
    		cu1216_writereg( state,0x05 , sTypeQAM3[bQAM].uLockthr) ;
    		// MSETH
    		cu1216_writereg( state,0x08 , sTypeQAM3[bQAM].uMseth) ;
   		// AREF
    		cu1216_writereg( state,0x09 , sTypeQAM3[bQAM].uAref) ;
    		// AGCREFNYQ
    		cu1216_writereg( state,TDA10023_AGCREFNYQ_IND , sTypeQAM3[bQAM].uAgcRefNyq) ;
    		// ERAGCNYQ_THD
    		cu1216_writereg( state,TDA10023_ERAGCNYQ_THD_IND , sTypeQAM3[bQAM].uErAgcNyqThd) ;
			
        	// GPR: program the modulation in QAM bits
        	cu1216_writereg_mask( state,0x00, 0x1c,sTypeQAM3[bQAM].uQam);
    		
	}//end else mk3
}

static void cu1216_set_IQ(struct dvb_frontend *fe, u8 bSI)
{
	struct cu1216_state *state = fe->demodulator_priv;

	// set the spectral inversion mode
	bSI = (bSI+1)%2;

	// write the ConfReg Register
	cu1216_writereg_mask(state, AC_CONF_IND, AC_CONF_INVIQ_BIT, (u8)(bSI << 5));
}

static void cu1216_set_gain(struct dvb_frontend *fe, u8  bGain)
{
	struct cu1216_state *state = fe->demodulator_priv;

	// write the gain
	cu1216_writereg_mask(state, AC_GAIN_IND, AC_GAIN_GNYQ_MSK, (u8)(bGain << 5));
}

static int cu1216_read_status(struct dvb_frontend *fe, fe_status_t *status)
{
	struct cu1216_state *state = fe->demodulator_priv;
	int sync;

	*status = 0;
	sync = cu1216_readreg (state, 0x11);
       
	if (sync & 2)
		*status |= FE_HAS_SIGNAL | FE_HAS_CARRIER;

	if (sync & 4)
		*status |= FE_HAS_SYNC | FE_HAS_VITERBI;

	if (sync & 8)
	{
		*status |= FE_HAS_LOCK;
               
		return 0;
	}
	return -1;
}

static int cu1216_read_errRate(struct dvb_frontend *fe)
{
	struct cu1216_state *state = fe->demodulator_priv;

	int cCkOffset = 0;
	int iErrRyt;

	// read offset
	cCkOffset = cu1216_readreg(state, 0x1d);

	// convert the value to long
	if (cCkOffset < 0)
		iErrRyt = (int)(0xFFFFFF00 | cCkOffset);
	else
		iErrRyt = (int)cCkOffset;

	// read DYN bit
	cCkOffset = cu1216_readreg(state, 0x03);

	// calculate the error in ppm
	iErrRyt *= 1000000;
	iErrRyt /= 262144;

	if (!(cCkOffset & 0x08))
		iErrRyt /= 2;

	return iErrRyt;
}

/*static int cu1216_read_quality(struct dvb_frontend *fe, u16 *quality)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8 tempval;
	fe_status_t tunerStatus;

	cu1216_read_status(fe, &tunerStatus);

	if (tunerStatus == 0) {
		*quality = 0;
		return 0;
	}

	tempval = cu1216_readreg(state, 0x18);

	if (tempval <= 5) {
		*quality = 98;
		return 0;

	} else if(tempval <= 10) {
		*quality = 108 - 2*tempval;//88
		return 0;
	}

	switch (state->params.u.qam.modulation) {
	case QPSK:
	case QAM_16:
		if (tempval <= 110)
			*quality = 60 + (110 - tempval) * 3 / 10;//0.3
		else if (tempval <= 120)
			*quality = 30 + (120 - tempval) * 30 / 10;
		else
			*quality = (255 - tempval) * 30 / 135;
		break;
	case QAM_32:
		if (tempval <= 72)
			*quality = 60 + (72 - tempval) * 15 / 31;//0.5
		else if (tempval <= 82)
			*quality = 30 + (83 - tempval) * 30 / 11;
		else
			*quality = (255 - tempval) * 30 / 172;
		break;
	default:
	case QAM_64:
		if (tempval <= 42)
			*quality = 60 + (42- tempval) * 7 / 8;//0.875
		else if (tempval <= 52)
			*quality = 30 + (52 - tempval) * 30 / 10;
		else
			*quality = (255-tempval)*30/203;
		break;
	case QAM_128:
		if (tempval <= 28)
			*quality = 60 + (28 - tempval) * 14 / 9;//1.5
		else if(tempval <= 34)
			*quality = 30 + (34 - tempval) * 30 / 6;
		else
			*quality = (255 - tempval) * 30 / 221;
		break;
	case QAM_256:
		if (tempval <= 18)
			*quality = 60 + (18 - tempval) * 7 / 2;//3.5
		else if (tempval <= 22)
			*quality = 30 + (22 - tempval) * 30 / 4;
		else
			*quality = (255 - tempval) * 30 / 233;
		break;
	}

    return 0;
}*/

static int cu1216_read_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8	tempagc;

	tempagc = cu1216_readreg(state, AC_VAGC1_IND);

	if (tempagc > 0xF0)
		*strength = 2;
	else if (tempagc > 0xE5)
		*strength = 4;
	else if (tempagc < 0x6E)
		*strength = 98;
	else if (tempagc < 0x96)
		*strength = 70 + (0x96 - tempagc) / 2 * 7 / 5;
	else if (tempagc < 0xCD)
		*strength = 14 + (0xCD - tempagc);
	else
		*strength = 2 + (0xE5 - tempagc) / 2;

	return  0;
}

static int cu1216_read_ber(struct dvb_frontend *fe, u32 *BERvalue)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8 tempval;

	tempval = cu1216_readreg(state, 0x16);

	tempval &= 0x0f;
	*BERvalue = tempval;
	*BERvalue <<=8;

	tempval = cu1216_readreg(state, 0x15);

	*BERvalue += tempval;
	*BERvalue <<=8;

	tempval = cu1216_readreg(state, 0x14);

	*BERvalue += tempval;

	tempval = cu1216_readreg(state, 0x10);

	switch (tempval & 0xc0) {
	default:
	case 0x00:  // 1,00E+05
		*BERvalue *= 80;
		break;

	case 0x40:  // 1,00E+06
		*BERvalue *= 10;
		break;

	case 0x80:  // 1,00E+07
		*BERvalue /= 1;
		break;
	case 0xc0:  // 1,00E+08
		*BERvalue /= 10;
		break;
	}
	return 0;
}

static int cu1216_read_snr (struct dvb_frontend *fe, u16 *SNRvalue)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8 tempagc;

	tempagc = cu1216_readreg(state, 0x18);

	*SNRvalue = tempagc;
	switch (state->params.u.qam.modulation) {
	case QAM_16:
		*SNRvalue = 195000 / (32 * (*SNRvalue) + 138) + 100;
		break;
	case QAM_32:
		*SNRvalue = 215000 / (40 * (*SNRvalue) + 500) + 135;
		break;
	default:
	case QAM_64:
		*SNRvalue = 210000 / (40 * (*SNRvalue) + 500) + 125;
		break;
	case QAM_128:
		*SNRvalue = 185000 / (38 * (*SNRvalue) + 400) + 138;
		break;
	case QAM_256:
		*SNRvalue = 180000 / (100 * (*SNRvalue) + 40) + 203;
		break;
	}
	return 0;
}



static int cu1216_read_ubk(struct dvb_frontend *fe, u32 *ubk)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8	puBytes[4];
	u32	puUBK;

	//----------------------
	// Implementation
	//----------------------
	puBytes[0] = cu1216_readreg(state, AC_CPTUNCOR_IND);
	puBytes[1] = cu1216_readreg(state, AC_BERLSB_IND);
	puBytes[2] = cu1216_readreg(state, AC_BERMID_IND);
	puBytes[3] = cu1216_readreg(state, AC_BERMSB_IND);


	puUBK = (puBytes[3] << 24) | (puBytes[2] << 16) | (puBytes[1] << 8) | puBytes[0];

	// mask the reset flag
	puUBK &= AC_CPTUNCOR_CPTU_MSK;

	// reset the counter is needed if there are uncors
	if (puUBK) {
		cu1216_writereg_mask(state, AC_RSCONF_IND, AC_RSCONF_CLBUNC_BIT, 0);
		cu1216_writereg_mask(state, AC_RSCONF_IND, AC_RSCONF_CLBUNC_BIT,AC_RSCONF_CLBUNC_BIT);
	}
	*ubk =  puUBK;

	return 0;
}

static void cu1216_clear_register(struct dvb_frontend *fe)
{
	struct cu1216_state *state = fe->demodulator_priv;

	cu1216_writereg(state, AC_CONF_IND, 0x6a);
}

static void cu1216_check_tunertype(struct dvb_frontend *fe)
{      
	struct cu1216_state *state = fe->demodulator_priv;

	type = cu1216_readreg(state, AC_IDENTITY_IND);
        dprintk( verbose, MANTIS_DEBUG, 1, " Detected TunerType:0x%x", type);
}

static int cu1216_init_none(struct dvb_frontend *fe)
{
	return 0;
}

static int cu1216_init(struct dvb_frontend *fe)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8  bByte;
	s32 lDeltaF;
	u32 AC_uSysClk;
	u8 puByte[3], uByte;
	
	u32	uSACLK,uSysClk;
        u32	uTUN_IF;
	u8	bFsampling,bModeDvbMcns,bBERdepth,bPolaPWM1,bPolaPWM2;
	u8 	bIFMax	,bIFMin;
	u8 	bTUNMax	,bTUNMin;
	u8 	bEqualType;
	u8	bSwDyn	,bSwStep;
	u8	bOUT_OClk1,bOUT_OClk2;
	u8	bOUT_bMSBFirst1,bOUT_bParaSer1;
	u8	bOUT_ModeABC1,bOUT_ParaDiv1;
	u8	bOUT_bMSBFirst2;

        if(type == MK1)
        {
		// calculate the system frequency
	 	AC_uSysClk  = OM5734_XTALFREQ_DEF * (OM5734_PLLMFACTOR_DEF + 1);
		AC_uSysClk /= (OM5734_PLLNFACTOR_DEF + 1) * (OM5734_PLLPFACTOR_DEF + 1);

		// PLL factors
		cu1216_writereg(state, AC_MDIV_IND, OM5734_PLLMFACTOR_DEF);
		cu1216_writereg(state, AC_NDIV_IND, OM5734_PLLNFACTOR_DEF);
		cu1216_writereg(state, AC_PLL_IND, OM5734_PLLPFACTOR_DEF);

		// add by ice_Deng 2004/01/06
		cu1216_writereg(state, AC_CONTROL_IND, 0x0d);

		// enable the PLL
		cu1216_writereg_mask(state, AC_PLL_IND, AC_PLL_BYPPLL_BIT, 0);

		// enable AGC2 and set PWMREF
		cu1216_writereg_mask(state, AC_AGCCONF2_IND, AC_AGCCONF2_ENAGC2_BIT, AC_AGCCONF2_ENAGC2_BIT);
		cu1216_writereg(state, AC_PWMREF_IND, AC_PWMREF_DEF);

		// use internal ADC
		cu1216_writereg(state, AC_ADC_IND, 0x31);
		cu1216_writereg(state, AC_ADC_IND, 0x31);
		// use only nyquist gain
		cu1216_writereg(state, AC_CLKCONF_IND, 0x0a);
		// set the acquisition to +/-480ppm
		cu1216_writereg(state, AC_CLKCONF_IND, 0x0a);
		// POS_AGC - not in data sheet

		cu1216_writereg(state, AC_AGCCONF1_IND, 0x23);
		cu1216_writereg(state, AC_AGCCONF1_IND, 0x23);

		if (AC_POLAPWM2_DEF)
			cu1216_writereg_mask(state, AC_AGCCONF2_IND, AC_AGCCONF2_PPWM2_BIT, AC_AGCCONF2_PPWM2_BIT);
		else
			cu1216_writereg_mask(state, AC_AGCCONF2_IND, AC_AGCCONF2_PPWM2_BIT, 0);

		// set the threshold for the IF AGC
		cu1216_writereg(state, AC_IFMAX_IND, AC_IFMAX_DEF);
		cu1216_writereg(state, AC_IFMIN_IND, 0); //AC_IFMIN_DEF150 ;


		// set the threshold for the TUN AGC
		cu1216_writereg(state, AC_TUNMAX_IND, AC_TUNMAX_DEF);
		cu1216_writereg(state, AC_TUNMIN_IND, AC_TUNMIN_DEF);


		// set the counter of saturation to its maximun size
		cu1216_writereg(state, AC_GAIN_IND, 0x23);

		// set the MPEG output clock polarity	
		// Added By IceDeng 12/15/2004 For Ts 188	
		cu1216_writereg(state, 0x12, 0xe1);
		cu1216_writereg(state, 0x12, 0xe1);

		//cyq channge star
		cu1216_writereg_mask(state, AC_POLA2_IND, AC_POLA2_POCLK2_BIT, AC_POLA2_POCLK2_BIT);

		// set the position of the central coeffcient
		cu1216_writereg_mask(state, AC_EQCONF1_IND, AC_EQCONF1_POSI_MSK, 0x70);

		// set the equalizer type
		if (AC_EQUALTYPE_DEF)
			cu1216_writereg_mask(state, AC_EQCONF1_IND, AC_EQCONF1_DFE_BIT, AC_EQUALTYPE_DEF-1);

		cu1216_writereg_mask(state, AC_EQCONF2_IND, AC_EQCONF2_SGNALGO_BIT, AC_EQCONF2_SGNALGO_BIT);


		lDeltaF  = (s32)(AC_uSysClk * 5 / 1000);
		lDeltaF /= -8;
		lDeltaF += (AC_TUNFI_DEF / 1000);
		lDeltaF *= 2048;
		lDeltaF /= (s32)(AC_uSysClk / 1000);

		cu1216_writereg(state, AC_DELTAF1_IND, (u8)lDeltaF);
		cu1216_writereg(state, AC_DELTAF2_IND, (u8)(((lDeltaF>>8) & 0x03) | AC_DELTAF2_ALGOD_BIT));

		// set the KAGC to its maximun value
		cu1216_writereg_mask(state, AC_AGCCONF1_IND, AC_AGCCONF1_KAGC_MSK, 0x03);

		// set carrier algorithm parameters and SELCAR
		bByte  = AC_SWEEP_DEF;
		bByte |= (u8)AC_CAROFFLENGTH_DEF;
		bByte |= (u8)(AC_CAROFFSTEP_DEF << 2);

		cu1216_writereg(state, AC_SWEEP_IND, bByte);

		// TS interface 1
		bByte = AC_INTP_DEF;
		if (AC_MSBFIRST1_DEF)
			bByte |= AC_INTP_MSBFIRST_BIT;
		if( AC_PARASER1_DEF)
			bByte |= AC_INTP_INTSEL_BIT;
		else
			bByte |= AC_INTP_MSBFIRST_BIT;	// set to 1 MSB if parallel

		if (AC_MODEAB1_DEF) {
			bByte |= AC_INTP_PARMOD_BIT;
			bByte |= (AC_PARADIV1_DEF << 4);
		}
		cu1216_writereg(state, AC_INTP_IND, bByte);
		cu1216_writereg_mask(state, AC_POLA2_IND, AC_POLA2_MSBFIRST2_BIT, AC_POLA2_MSBFIRST2_BIT);

		// set the BER depth
		cu1216_writereg_mask(state, AC_RSCONF_IND, AC_RSCONF_PVBER_MSK, (AC_BERDEPTH_DEF<< 6));
        }
        else
        {    		
		// calculate the system frequency
		uSysClk  = OM5734_XTALFREQ_DEF * (OM5734_PLLMFACTOR_DEF + 1);
	    	uSysClk /= (OM5734_PLLNFACTOR_DEF + 1) * (OM5734_PLLPFACTOR_DEF + 1);

		// Calculate the sampling clock
		bFsampling=1;
		if(bFsampling)	
		{
		    // high sampling clock
		    uSACLK = uSysClk;
		}
		else
		{
		    // low sampling clock
		    uSACLK = uSysClk/2;
		}
		// read the PLL M,N,P values
		puByte[0] = cu1216_readreg( state,AC_MDIV_IND );
		puByte[1] = cu1216_readreg( state,AC_NDIV_IND );

		uByte = cu1216_readreg( state,AC_CONF_IND );

		// change the PLL M,N,P values and FSAMPLING only if different
		if((puByte[0] != OM5734_PLLMFACTOR_DEF) || 
		   ((puByte[1] & TDA10023_PLL2_PDIV_MSK) != OM5734_PLLNFACTOR_DEF) ||
		   (((puByte[1] & TDA10023_PLL2_NDIV_MSK)>>6) != OM5734_PLLPFACTOR_DEF) ||
		   (((uByte & TDA10023_GPR_FSAMPLING_BIT) >> 5) != bFsampling) )
		{
		    u8 uPLL3;

		    // disable the PLL
		    uPLL3 = cu1216_readreg( state,AC_PLL_IND ) ;

		    uPLL3 |= (TDA10023_PLL3_BYPPLL_BIT | TDA10023_PLL3_PDPLL_BIT);

		    cu1216_writereg( state,AC_PLL_IND, uPLL3);
		    // !!!! NOTE !!!! : When the PLL is disable, TDA10023 registers can't be read
		    // Therefore, SY_Read and SY_WriteBit functions must not be called
		  
		    // PLL factors 
		    puByte[0]  = OM5734_PLLMFACTOR_DEF;
		    puByte[1]  = OM5734_PLLNFACTOR_DEF & 0x3F;
		    puByte[1] |= OM5734_PLLPFACTOR_DEF << 6;

		    // write the PLL registers with PLL bypassed		   		    
		    cu1216_writereg( state,AC_MDIV_IND, puByte[0]);
		    cu1216_writereg( state,AC_NDIV_IND, puByte[1]);

		    // Set FSAMPLING
		    if(bFsampling)
		    {
			uByte = TDA10023_GPR_FSAMPLING_BIT | TDA10023_GPR_CLBS2_BIT | TDA10023_GPR_CLBS_BIT;
			cu1216_writereg( state,AC_CONF_IND, uByte);

		    }
		    else
		    {

		        uByte = TDA10023_GPR_CLBS2_BIT | TDA10023_GPR_CLBS_BIT;
				cu1216_writereg( state,AC_CONF_IND, uByte);

		    }

		    // enable the PLL
		    uPLL3 &= ~(TDA10023_PLL3_BYPPLL_BIT | TDA10023_PLL3_PDPLL_BIT);
		    cu1216_writereg(state,AC_PLL_IND, uPLL3);

		}

		// Set DVB/MCNS mode
		bModeDvbMcns=0;
		bBERdepth=2;
		if (bModeDvbMcns)
		{
		    // MCNS mode
		    cu1216_writereg_mask( state,AC_RESET_IND,TDA10023_RESET_DVBMCNS_BIT, 
		        TDA10023_RESET_DVBMCNS_BIT);

		    // set the BER depth
		    cu1216_writereg_mask( state,TDA10023_RSCFG_IND, 0x0c, bBERdepth << 2);
		}
		else
		{
		    // DVB mode
		    cu1216_writereg_mask( state,AC_RESET_IND,TDA10023_RESET_DVBMCNS_BIT, 0);

		    // set the BER depth
		    cu1216_writereg_mask( state,AC_RSCONF_IND, 0xc0, bBERdepth << 6);

		}

		// set GAIN1 bit to 0
		uByte = 0x82;
		cu1216_writereg( state,AC_GAIN_IND, uByte);

		// set the acquisition to +/-480ppm
		cu1216_writereg_mask( state,AC_CLKCONF_IND, TDA10023_CLKCONF_DYN_BIT,TDA10023_CLKCONF_DYN_BIT);

		// TRIAGC, POSAGC, enable AGCIF
#ifdef _MULTIFE_CU1216
		cu1216_writereg_mask( state,AC_AGCCONF2_IND,
		    TDA10023_AGCCONF2_TRIAGC_BIT | TDA10023_AGCCONF2_POSAGC_BIT | TDA10023_AGCCONF2_ENAGCIF_BIT,
		    0                            | TDA10023_AGCCONF2_POSAGC_BIT) ;
#else
		cu1216_writereg_mask( state,AC_AGCCONF2_IND,
			TDA10023_AGCCONF2_TRIAGC_BIT | TDA10023_AGCCONF2_POSAGC_BIT | TDA10023_AGCCONF2_ENAGCIF_BIT,
			0							 | TDA10023_AGCCONF2_POSAGC_BIT | 0);

#endif
		// set the AGCREF
		uByte = 80;
		cu1216_writereg( state,AC_AGCREF_IND, uByte);


		// set SACLK_OFF
		cu1216_writereg_mask( state,0x1e,TDA10023_CONTROL_OLDBYTECLK_BIT ,TDA10023_CONTROL_OLDBYTECLK_BIT );

		// program CS depending on SACLK and set GAINADC
		uByte = 0xc8;

		cu1216_writereg( state,AC_ADC_IND, uByte);

		// set the polarity of the PWM for the AGC
		bPolaPWM1=0;
		if (bPolaPWM1)
		{
		    cu1216_writereg_mask( state,AC_AGCCONF2_IND, TDA10023_AGCCONF2_PPWMTUN_BIT, TDA10023_AGCCONF2_PPWMTUN_BIT);
		}
		else
		{
		    cu1216_writereg_mask(  state,AC_AGCCONF2_IND, TDA10023_AGCCONF2_PPWMTUN_BIT, 0);
		}
		bPolaPWM2=0;
		if (bPolaPWM2)
		{
		    cu1216_writereg_mask( state,AC_AGCCONF2_IND, TDA10023_AGCCONF2_PPWMIF_BIT, TDA10023_AGCCONF2_PPWMIF_BIT);
		}
		else
		{
		    cu1216_writereg_mask( state,AC_AGCCONF2_IND, TDA10023_AGCCONF2_PPWMIF_BIT, 0);
		}
		// set the threshold for the IF AGC
		bIFMax=255;
		bIFMin=0;
		puByte[0] = bIFMax;
		puByte[1] = bIFMin;

		cu1216_writereg( state,AC_IFMAX_IND, puByte[0]);
		cu1216_writereg( state,AC_IFMIN_IND , puByte[1]);

		// set the threshold for the TUN AGC
		bTUNMax=255;
		bTUNMin=0;
		puByte[0] = bTUNMax;
		puByte[1] = bTUNMin;

		cu1216_writereg(state, AC_TUNMAX_IND , puByte[0]);
		cu1216_writereg(state, AC_TUNMIN_IND , puByte[1]);

		// configure the equalizer
		bEqualType=2;
		if(bEqualType == 0)
		{
		    // disable the equalizer
		    uByte = 0x70;
		}
		else
		{
		    // enable the equalizer and set the DFE bit
		    uByte = 0x70 | TDA10023_EQCONF1_ENADAPT_BIT | 
		            TDA10023_EQCONF1_ENEQUAL_BIT |(bEqualType-1);
		}
		cu1216_writereg( state,AC_EQCONF1_IND , uByte);


		cu1216_writereg_mask( state,AC_EQCONF2_IND,
		    TDA10023_EQCONF2_SGNALGO_BIT | TDA10023_EQCONF2_STEPALGO_BIT,
		    TDA10023_EQCONF2_SGNALGO_BIT | TDA10023_EQCONF2_STEPALGO_BIT);

		// set ALGOD and deltaF
		uTUN_IF=36130000;

		if(bFsampling)
		{
		    // FSAMPLING = 1 - high sampling clock
		    // SACLK = Sysclk  (SACLK max = 72MHz)
		    lDeltaF  = (long)(uTUN_IF/1000);
		    lDeltaF *= 32768;                                   // 32768 = 2^20/32
		    lDeltaF += (long)(uSysClk/500);
		    lDeltaF /= (long)(uSysClk/1000);
		    lDeltaF -= 53248;                                   // 53248 = (2^20/32) * 13/8
		}
		else
		{
		    // FSAMPLING = 0 - low sampling clock
		    // SACLK = Sysclk/2 (SACLK max = 36MHz)
		    lDeltaF  = (long)(uTUN_IF/1000);
		    lDeltaF *= 32768;                                   // 32768 = 2^20/32
		    lDeltaF += (long)(uSysClk/1000);
		    lDeltaF /= (long)(uSysClk/2000);
		    lDeltaF -= 40960;                                   // 53248 = (2^20/32) * 5/4
		}

		puByte[0] = (u8)lDeltaF;
		puByte[1] = (u8)(((lDeltaF>>8) & 0x7F) | TDA10023_DELTAF_MSB_ALGOD_BIT);

		cu1216_writereg( state,AC_DELTAF1_IND , puByte[0]);
		cu1216_writereg( state,AC_DELTAF2_IND , puByte[1]);

		// set the KAGCIF and KAGCTUN to acquisition mode
		uByte = 0x93;//0x93 cyqagc
		cu1216_writereg( state,AC_AGCCONF1_IND  , uByte);


		// set carrier algorithm parameters
		bSwDyn=0;//7
		bSwStep=1;
		uByte  = 0x82;
		uByte |= bSwDyn << 4;
		uByte |= bSwStep << 2;
		cu1216_writereg( state,AC_SWEEP_IND  , uByte);

		// set the MPEG output clock polarity
		bOUT_OClk1=1;
		bOUT_OClk2=1;
		if(bOUT_OClk1)
		{
		    cu1216_writereg_mask( state,
		    AC_POLA1_IND, TDA10023_INTP1_POCLKP_BIT, TDA10023_INTP1_POCLKP_BIT);
		}		
		else
		{
		    cu1216_writereg_mask(state,
		    AC_POLA1_IND, TDA10023_INTP1_POCLKP_BIT, 0);
		 }
		if(bOUT_OClk2)
		{
		    cu1216_writereg_mask(state,
		    AC_POLA2_IND, TDA10023_INTS1_POCLKS_BIT, TDA10023_INTS1_POCLKS_BIT);
		}
		else
		{
		    cu1216_writereg_mask(state,
		    AC_POLA2_IND, TDA10023_INTS1_POCLKS_BIT, 0);
		}

		// TS interface 1
		uByte = 0;
		bOUT_bMSBFirst1=0;
		bOUT_bParaSer1=0;
		if(bOUT_bMSBFirst1)
		    uByte |= TDA10023_INTP2_MSBFIRSTP_BIT;
		if(bOUT_bParaSer1)
		{
		    // SERIAL
		    uByte |= 0x03;
		}
		else
		{
		    // PARALLEL
		    // set to 1 MSB if parallel
		    uByte |= TDA10023_INTP2_MSBFIRSTP_BIT;
			bOUT_ModeABC1=0;
			bOUT_ParaDiv1=0;
		    if(bOUT_ModeABC1 == 0)
		    {
		        // PARALLEL mode A
		        uByte |= 0x00;
		    }
		    else if(bOUT_ModeABC1 == 1) 
		    {
		        // PARALLEL mode B
		        uByte |= 0x01;
		        uByte |= bOUT_ParaDiv1 << 4;
		    }
		    else //if(bOUT_ModeABC1 == 2)
		    {
		        // PARALLEL mode C
		        uByte |= 0x02;
		    }
		}
		cu1216_writereg(state, 0x20  , uByte);



		// TS interface 2
		bOUT_bMSBFirst2=0;
		if(bOUT_bMSBFirst2)
		{
		    cu1216_writereg_mask( state,AC_CONTROL_IND, TDA10023_INTPS_MSBFIRSTS_BIT,
		    TDA10023_INTPS_MSBFIRSTS_BIT);
		}
		else
		{
		    cu1216_writereg_mask( state,AC_CONTROL_IND, TDA10023_INTPS_MSBFIRSTS_BIT, 0);
		}
		// disable the tri state of the outputs
		cu1216_writereg_mask( state,AC_CONTROL_IND, TDA10023_INTPS_TRIP_BIT | TDA10023_INTPS_TRIS_BIT, 0);


		// Soft reset
		cu1216_writereg_mask(state, 0x00, TDA10023_GPR_CLBS2_BIT | TDA10023_GPR_CLBS_BIT, TDA10023_GPR_CLBS2_BIT | 0);
	}     
	return 0;
}

static void delay_ms_interruptible(u32 ms)
{
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(HZ * ms / 100);
}

static void delay_us_interruptible(u32 us)
{
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(HZ * us / 10000);
}

static int  tda10023_set_tuner(struct dvb_frontend *fe, 
	          struct dvb_frontend_parameters *params)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u32 uFreqPll,uRfProg;
	u8  pTunerReg[6];
	struct i2c_msg msg = {
		.addr = 0xc0 >> 1,
		.flags = 0,
		.buf = pTunerReg,
		.len = 4
 	};

	dprintk(verbose, MANTIS_ERROR, 1, "Freq = %d", params->frequency);

	cu1216_writereg(state, 0x02, 0x93);
	// call the tuner function with the tuner IF

	// calculate N0-N14
	uFreqPll = params->frequency;//*1000;
	uFreqPll +=36130000;
	uFreqPll += 62500/2;
	uRfProg  = uFreqPll;
	uFreqPll /= 62500;

	// real frequency programmed
	//  uRfProg  = uFreqPll * uPllStep;
	//-------------
	// byte 0 and 1
	//-------------
	// divider ratio
	pTunerReg[0] = (u8)(uFreqPll >> 8);
	pTunerReg[1] = (u8)uFreqPll;
	//-------
	// byte 2
	//-------
	//	pTunerReg[2] &= 0xF9;
	pTunerReg[2] = 0xCE;
	pTunerReg[2] &= 0xF8;
	pTunerReg[2] |= 0x06;
	//-------
	// byte 3
	//-------
	//	pTunerReg[3] &= 0xF8;
	pTunerReg[3] = 0x00;

	if (uRfProg < (160000000 + 36125000)) 
		pTunerReg[3] |= 0x01;
	else if (uRfProg > (446000000 + 36125000)) // high band
		pTunerReg[3] |= 0x04;
	else	// mid band
		pTunerReg[3] |= 0x02;
	//-------
	// byte 4 = byte 2 with T2T1T0 = 011 for AB Byte Xfer
	//-------
	pTunerReg[4] = pTunerReg[2] & 0xC7; // mask first	
	pTunerReg[4] |= 0x18; // T2T1T0 = 011 

	//-------
	// byte 5 = AB byte
	//-------
	pTunerReg[5] = 0x00 | 0 << 7 | 2 << 4;

	// write in the tuner
	// first, CB + AB bytes
	tda10023_writereg(state, pTunerReg[4], pTunerReg[5]);

	// then all the other bytes, DB1, DB2, CB and BB
	if (i2c_transfer(state->i2c, &msg, 1) < 0) {
		printk("%s tuner not ack!\n", __FUNCTION__);
		return -EIO;
	}
       tda10023_writereg(state, pTunerReg[4], pTunerReg[5]);
	   
	msleep(100);
	return 0;

}

static int cu1216_set_parameters(struct dvb_frontend *fe, struct dvb_frontend_parameters *params)
{
	struct cu1216_state *state = fe->demodulator_priv;

	u8 i;
	u8 QamSize = 0;
	u32 ErrRate[3];
	fe_status_t value;
	int status = -EINVAL;
	u8  li_Iq, li_oldIq = 0, uc_Gain, uc_oldGain = 0;

	/*printk("[%s]:frequency = %d , symbol = %d , qam = %d .\n",
		__func__,
		params->frequency , params->u.qam.symbol_rate,
		params->u.qam.modulation);*/

       //get tuner type for mk1 mk3
	cu1216_check_tunertype(fe);
	if(type == MK1)
	{
                
        	dprintk( verbose, MANTIS_DEBUG, 1, " MK1");
		switch (params->u.qam.modulation) {
		case QPSK   :
			QamSize = 0;
			break;
		case QAM_16 :
			QamSize = 1;
			break;
		case QAM_32 :
			QamSize = 2;
			break;
		case QAM_64 :
			QamSize = 3;
			break;
		case QAM_128:
			QamSize = 4;
			break;
		case QAM_256:
			QamSize = 5;
			break;
		default :
			printk("[cu1216_set_parameters]:QAM set error!\n");
			break;
		}

		if (li_oldIq >= 2)
			li_oldIq = 0;
		
		//reset tuner
		//state->config->fe_reset(fe);

		//To clear the Registers in TDA10021HT
		cu1216_clear_register(fe);

		//Write Frequency into tuner
		lock_tuner(state);
		state->config->pll_set(fe, params);
		unlock_tuner(state);

		//mdelay(20);
		delay_ms_interruptible(10);

		//Second step to init the cu1216ht's registers
		cu1216_init(fe);

		//Write Symborate
		cu1216_set_symbolRate(fe, params->u.qam.symbol_rate / 1000);

		//Write QAM
		cu1216_set_QAM(fe, QamSize);
	
		for (i = li_oldIq; i < li_oldIq + 2; i++) {
			li_Iq = i % 2;

			for (uc_Gain = 1; uc_Gain < 4; uc_Gain++) {
				cu1216_set_IQ(fe, li_Iq);

				cu1216_set_gain(fe, uc_Gain);

				//udelay(50);
				delay_us_interruptible(5);

				if (cu1216_read_status(fe, &value) == 0) {

					li_oldIq   = li_Iq;
					uc_oldGain = uc_Gain;
					ErrRate[0] = cu1216_read_errRate(fe);

					if (uc_Gain < 3) {
						cu1216_set_gain(fe, uc_Gain+1);
						//udelay(50);
						delay_us_interruptible(5);
						ErrRate[1] = cu1216_read_errRate(fe);

						if (ErrRate[0] > ErrRate[1]) {
							cu1216_set_gain(fe , uc_Gain);
							//udelay(50);
							delay_us_interruptible(5);

						} else {
							uc_oldGain = uc_Gain + 1;
							uc_Gain = uc_Gain + 1;

							if (uc_Gain < 3) {
								cu1216_set_gain(fe, uc_Gain + 1);

								//udelay(50);
								delay_us_interruptible(5);
								ErrRate[2] = cu1216_read_errRate(fe);

							if (ErrRate[1] > ErrRate[2]) {
								cu1216_set_gain(fe , uc_oldGain);

								//udelay(50);
								delay_us_interruptible(5);
							} else {
								uc_oldGain = uc_Gain + 1;
							}
						}
					}
				}
				goto ret;
			}
		}
	}

	status = -1;
	}else{
        	dprintk( verbose, MANTIS_DEBUG, 1, " MK3");
		switch(params->u.qam.modulation)
		{
                    //case QPSK : QamSize = 0; break;
	             case QAM_16 : QamSize = 1; break;
	             case QAM_32 : QamSize = 2; break;
	             case QAM_64 : QamSize = 3; break;
	             case QAM_128: QamSize = 4; break;
	             case QAM_256: QamSize = 5; break;
	             default : QamSize = 3; break;
		}   
		//reset tuner
		state->config->fe_reset(fe);	

		//Write Frequency into tuner
	       lock_tuner(state);
	       tda10023_set_tuner(fe, params);
	       unlock_tuner(state);
		
		//Second step to init the cu1216ht's registers
	       cu1216_init(fe);	

		//Write Symborate
	       cu1216_set_symbolRate(fe, params->u.qam.symbol_rate / 1000);

		//Write QAM
	       cu1216_set_QAM(fe, QamSize);

		//cu1216_wait(50);
		delay_ms_interruptible(50);
		cu1216_writereg(state, AC_AGCCONF1_IND, 0x9B);//0x9B hick track mode
		
		//AC_WriteSI(  2);
		cu1216_writereg_mask(state, AC_CARCONF_IND, 
		         TDA10023_CARCONF_AUTOINVIQ_BIT | TDA10023_CARCONF_INVIQ_BIT,
		         (u8)(2<<5));			

		//Soft reset
		cu1216_writereg_mask(state,AC_CONF_IND,
                   TDA10023_GPR_CLBS2_BIT | TDA10023_GPR_CLBS_BIT,
                   TDA10023_GPR_CLBS2_BIT | 0);

		return 0 ;
	}
ret:
	state->params = *params;
	return status ;
}

static int cu1216_get_frontend(struct dvb_frontend *fe, struct dvb_frontend_parameters *p)
{
	struct cu1216_state *state = fe->demodulator_priv;
	int sync;
	s8 afc = 0;

	sync = cu1216_readreg(state, 0x11);
	afc = cu1216_readreg(state, 0x19);
	if (verbose) {
	/* AFC only valid when carrier has been recovered */
		printk(sync & 2 ? "DVB: TDA10021(%d): AFC (%d) %dHz\n" :
		       "DVB: TDA10021(%d): [AFC (%d) %dHz]\n",
		       state->frontend.dvb->num, afc,
		       - ((s32)p->u.qam.symbol_rate * afc) >> 10);
	}

	p->inversion = HAS_INVERSION(state->reg0) ? INVERSION_ON : INVERSION_OFF;
	p->u.qam.modulation = ((state->reg0 >> 2) & 7) + QAM_16;

	p->u.qam.fec_inner = FEC_NONE;
	p->frequency = ((p->frequency + 31250) / 62500) * 62500;

	if (sync & 2)
		p->frequency -= ((s32)p->u.qam.symbol_rate * afc) >> 10;

	return 0;
}


static int cu1216_sleep(struct dvb_frontend *fe)
{
	struct cu1216_state *state = fe->demodulator_priv;

	cu1216_writereg (state, 0x1b, 0x02);  /* pdown ADC */
	cu1216_writereg (state, 0x00, 0x80);  /* standby */

	return 0;
}

static void cu1216_release(struct dvb_frontend *fe)
{
	struct cu1216_state *state = fe->demodulator_priv;
	kfree(state);
}

static struct dvb_frontend_ops cu1216_ops;

struct dvb_frontend *cu1216_attach(const struct cu1216_config *config,
				   struct i2c_adapter *i2c)
{
	struct cu1216_state *state = NULL;

	/* allocate memory for the internal state */
	state = kmalloc(sizeof (struct cu1216_state), GFP_KERNEL);
	if (state == NULL)
		goto error;

	/* setup the state */
	state->config = config;
	state->i2c = i2c;
	memcpy(&state->ops, &cu1216_ops, sizeof (struct dvb_frontend_ops));

	/* check if the demod is there */
	if ((cu1216_readreg(state, 0x1a) & 0xf0) != 0x70) //reg 0x1a == 0x7c
		goto error;

	/* create dvb_frontend */
	state->frontend.ops = state->ops;
	state->frontend.demodulator_priv = state;
	return &state->frontend;

error:
	kfree(state);
	return NULL;
}

static struct dvb_frontend_ops cu1216_ops = {

	.info = {
		    .name = "Philips CU1216 DVB-C",
		    .type = FE_QAM,
		    .frequency_stepsize = 62500,
		    .frequency_min = 51000000,
		    .frequency_max = 858000000,
		    .symbol_rate_min = (XIN / 2) / 64,     /* SACLK/64 == (XIN/2)/64 */
		    .symbol_rate_max = (XIN / 2) / 4,      /* SACLK/4 */
		    .caps = 0x400	   | //FE_CAN_QAM_4
			    FE_CAN_QAM_16  |
			    FE_CAN_QAM_32  |
			    FE_CAN_QAM_64  |
			    FE_CAN_QAM_128 |
			    FE_CAN_QAM_256 |
			    FE_CAN_FEC_AUTO
	},

	.release = cu1216_release,
	.init = cu1216_init_none,
	.sleep = cu1216_sleep,
	.set_frontend = cu1216_set_parameters,
	.get_frontend = cu1216_get_frontend,
	.read_status = cu1216_read_status,
	.read_ber = cu1216_read_ber,
	.read_signal_strength = cu1216_read_strength,
	.read_snr = cu1216_read_snr,
	.read_ucblocks = cu1216_read_ubk,
};


MODULE_DESCRIPTION("Philips CU1216 DVB-C demodulator driver");
MODULE_AUTHOR("Ralph Metzler, Holger Waechtler, Markus Schulz");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(cu1216_attach);
