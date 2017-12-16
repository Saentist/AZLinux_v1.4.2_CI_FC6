/*
 * stb0899.c
 *
 * ST DVB-S/DVB-S2 Frontend Driver 
 *
 * Copyright (C) 2001 fnbrd <fnbrd@gmx.de>
 *             & 2002-2004 Andreas Oberritter <obi@linuxtv.org>
 *             & 2003 Wolfram Joost <dbox2@frokaschwei.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


 #include <linux/kernel.h>
 #include <linux/module.h>
 #include <linux/string.h>
 #include <linux/slab.h>
 #include <linux/i2c.h>

 #include "stb0899.h"
 #include "stb0899_common.h"
 #include "stb0899_drv.h"
 #include "stb0899_chip.h"

int debug = 1;
module_param(debug, int, 0644);

//int g_DvbsMode = 0;

extern FE_STB0899_LOOKUP_t FE_STB0899_CN_LookUp;
extern FE_STB0899_LOOKUP_t FE_STB0899_RF_LookUp;
extern FE_STB0899_LOOKUP_t FE_STB0899_DVBS2RF_LookUp;


struct stb0899_state {
	struct i2c_adapter		*i2c_adap;
	const struct stb0899_config	*config;
	struct dvb_frontend		frontend;
	u16				g_DvbsMode;
	FE_STB0899_Handle_t	 	m_Handle;
	u8				signal;
	fe_sec_mini_cmd_t 		minicmd;
	fe_sec_tone_mode_t 		tone;
};

int stb0899_set_standard(struct dvb_frontend* fe, u16 *standard)
{
	struct stb0899_state *state = fe->demodulator_priv;
	
	//printk(KERN_ERR "stb0899_set_standard = %d \n", *standard);
	state->g_DvbsMode = *standard;

	return 0;
}

int STB0899_Lock_Channel (struct stb0899_state * state, u32 freq, u32 symb )
{	
	FE_STB0899_InternalParams_t *pInlParams;
	FE_STB0899_SearchParams_t Params;
	FE_STB0899_SearchResult_t Results;
	FE_STB0899_Error_t ChipError;
	FE_STB0899_SignalInfo_t pInfo;

	memset(&Params, 0, sizeof(Params));
	memset(&Results, 0, sizeof(Results));
	memset(&ChipError, 0, sizeof(ChipError));
	memset(&pInfo, 0, sizeof(pInfo));

	/* Search parameters */
	//printk(KERN_ERR "freq=%ld,symb=%ld\n",freq,symb);
	Params.Frequency = freq;      	/*tuner frequency (in KHz)*/
	Params.SymbolRate = symb;     	/*symbol rate  (in bds)*/
	Params.SearchRange = 40000000;	/*range of the search (in Hz)*/
	Params.IQ_Inversion = FE_IQ_AUTO;	/* I,Q Inversion */

	if (state->g_DvbsMode==1)
	{
		Params.Standard	= FE_DVBS2_STANDARD;
		Params.Modulation = FE_MOD_8PSK;
	}
	else if (state->g_DvbsMode==2)
	{
		Params.Standard = FE_DSS_STANDARD;
		Params.Modulation = FE_MOD_QPSK;
	}
	else
	{
		Params.Standard = FE_DVBS1_STANDARD;
		Params.Modulation = FE_MOD_QPSK;
	}

	pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;

	/* Launch the search algorithm	*/
	ChipError = FE_STB0899_Search(state->m_Handle, &Params, &Results);

	if (pInlParams->hTuner->Chip->ChipError || pInlParams->hDemod->ChipError)
	{
	 	return -1;
	}
	else
	{
		FE_STB0899_GetSignalInfo(state->m_Handle, &pInfo);
		
		if (pInfo.Locked)
		{
			printk(KERN_ERR "^_^ Transponder locked\r\n");
			return 0;
		}
		else
		{
			printk(KERN_ERR ":( Transponder not found\r\n");
			return -1;
		}
	}
}

int st0899_initialize(struct dvb_frontend* fe)
{
	struct stb0899_state *state = fe->demodulator_priv;
	
	STB0899_InitParams_t	DemodInit;	/*Demod init params*/
	TUNER_InitParams_t	TunerInit;	/*Tuner init params*/
	STCHIP_Info_t		DemodChip;	/*Demod chip init params*/
	STCHIP_Info_t		TunerChip;	/*Tuner chip init params*/
	STCHIP_Info_t		LnbInit;
	/* Front-end (Demod + tuner) init params*/
	FE_STB0899_InitParams_t	Init;	
	
	u32 STB0899Val[STB0899_NBREGS] = {
		0x30,0x32,0x80,0x4,0x0,0x0,0x0,0x20,0x99,0xa8,0xb,0x11,0xa,0x5,0x0,0x0,0x0,0x0,0xfe,
		0x3,0x7c,0xf4,0xf3,0xfc,0xff,0xff,0x0,0x88,0x5c,0x0,0x0,0x0,0x0,0x33,0x6d,0x90,0x60,  //5C <-> F8
		0x0,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
		0x82,0x82,0x82,0x82,0x82,0xb8,0xba,0x1c,0x82,0x91,0x82,0x7e,0x82,0x82,0x82,0x20,0x82,
		0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x17,0x2,0x0,0x1,0x20,0x0,0xb,0x0,0x0,0x0,0xa,0x2,
		0x3ed097b6,0x3792,0x3fba,0x7aa,0x201,0xf,0x3fb4a20,0x200c17,0x16,0x0,0x0,0x0,0x3ed097b6,
		0x0,0x0,0xf6cdc01,0x0,0x3993,0xd3c6f,0x0,0x0,0x238e38e,0x0,0x0,0x0,0x0,0x0,0x0,0x39488000,
		0x1,0x2,0x0,0x17d3,0x0,0x1,0x7,0x2,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0xffff,0x101,0xfefe,0x404,0xcfcf,0xbebe,0xc2c2,0xc0c0,0xc1c1,0xc1c1,0xc1c1,
		0xc0c1,0xc1c1,0xc0c1,0xc1c1,0xc0c1,0xc1c1,0xc0c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,
		0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,
		0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0xc1c1,0x1,0x5654,0x0,0x20019,
		0x4b3237,0x3dd17,0x8008,0x2a3106,0x6140a,0x8000,0x0,0x0,0x471,0x17b0465,0x2,0x196464,0x603,
		0x2046666,0x10046583,0x10404,0x2aa8a,0x0,0x1,0x500,0x28a0a0,0x0,0x800c17,0xd01,0x0,0x54802,
		0x0,0x0,0x0,0x0,0x0,0x400,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0xffa,0xffd,0xfff,0xffe,0x0,0x36b,0x5,0x1,0x1,0xfff,0xffd,0xfff,0x1,0xfff,0x1,0x1,0x1,0x2,0xffe,
		0xfff,0x0,0xffd,0x0,0xc9,0x1,0x10,0x23,0x4e,0x34,0x84,0xf7,0x87,0x94,0x41,0xf1,0xe3,0xb4,0x10,
		0x30,0xfd,0xff,0xc,0xf,0x6c,0x80,0x6,0x0,0x30,0x7f,0x0,0xbc,0xea,0x31,0x2b,0x80,0x1d,0xa6,0x2f,
		0x68,0x40,0x2f,0x68,0x40,0x2,0xff,0x4,0x5,0x2,0xfd,0x3,0x7,0x8,0xf5,0x0,0x0,0x86,0x2a,0x0,0x0,0x0,
		0x0,0xa,0xad,0x6,0x1,0xb0,0x7a,0x58,0x38,0x34,0x24,0xff,0x19,0xb1,0x42,0x41,0x12,0xc,0x0,0x0,0x69,
		0x0,0x2,0x0,0x0,0x1b,0xb3,0x0,0x0,0xbc,0xcc,0xbd,0x90,0xb6,0x95,0x8d,0x27,0x3,0x5c,0x19,0x48,0x0,
		0x0,0x0,0x77,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf0,0x2,0x45,0x60,0xe3,0x0,0x47,0x5,0x18,
		0x19,0x2b,0x0,0x1,0x0,0x26,0x8,0xb4,0x28,0x4b5,0xb4b,0x78,0x1e0,0xa8c0,0xc,0x1,0x545,
		0x40,0x0,0x0,0x8,0x8,0x4db,0x0,0x8,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xc0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
		0x0,0x0,0x0,0x30
	};

	u32 DefSTV6100Val[STB6100_NBREGS]=
	{
		0x81,0x94,0x4a,0x26,0x3c,0x3b,0xcd,0xdc,0x8f,0x4d,0xeb,0xde
	};
	
	memset(&DemodInit, 0, sizeof(DemodInit));
	memset(&TunerInit, 0, sizeof(TunerInit));
	memset(&DemodChip, 0, sizeof(DemodChip));
	memset(&TunerChip, 0, sizeof(TunerChip));
	
	/* Demodulator */
	DemodInit.Chip = &DemodChip;
	DemodInit.NbDefVal = STB0899_NBREGS;
	DemodInit.DefVal = STB0899Val;
   	DemodInit.Chip->RepeaterHost = NULL;
   	DemodInit.Chip->RepeaterFn = NULL;
   	DemodInit.Chip->Repeater = FALSE;
	DemodInit.Chip->I2cAddr = state->config->demod_address;
	DemodInit.Chip->I2c_adap = state->i2c_adap;
	strcpy(DemodInit.Chip->Name,"DEMOD");
	
	/* Tuner  */
	TunerInit.Chip = &TunerChip;
	TunerInit.NbDefVal = STB6100_NBREGS;
	TunerInit.DefVal = DefSTV6100Val;
	TunerInit.Chip->RepeaterFn = STB0899_RepeaterFn;
	TunerInit.Chip->Repeater = TRUE;
	TunerInit.Chip->I2cAddr = 0xc0 >> 1;
	TunerInit.Chip->I2c_adap = state->i2c_adap;
	strcpy(TunerInit.Chip->Name,"STB6100");

	/*Lnbp21*/
	LnbInit.RepeaterHost = NULL;
	LnbInit.RepeaterFn = NULL;
	LnbInit.Repeater = FALSE;
	LnbInit.I2cAddr = 0x10 >> 1;
	LnbInit.I2c_adap = state->i2c_adap;
	strcpy(LnbInit.Name,"LNBP21");
	
	/* Front-end */
	if (state->g_DvbsMode==1)
		Init.Standard = FE_DVBS2_STANDARD;
	else if (state->g_DvbsMode==2)
		Init.Standard = FE_DSS_STANDARD;
	else
		Init.Standard = FE_DVBS1_STANDARD;
		
	Init.Clock = FE_PARALLEL_CLOCK; /*Data mode*/
	Init.Parity = FE_PARITY_ON; 	/*Data parity*/
	Init.STB0899Init = &DemodInit; 	/*Demod initial params*/
	Init.TunerInit = &TunerInit; 	/*Tuner initial params*/
	Init.LnbInit = &LnbInit;	/*Lnb   initial params*/
	
	state->m_Handle = FE_STB0899_Init(&Init);
	if(state->m_Handle != 0)
	{
		if ((TunerInit.Chip->ChipError) || (DemodInit.Chip->ChipError))
		{
			printk(KERN_ERR "STB0899 initialize finished Dmmodulator Error=%d", DemodInit.Chip->ChipError);
		}
		else
		{
			printk(KERN_ERR "STB0899 initialize success \n");
		}
		return 0;
	}
	else
	{
		printk(KERN_ERR "STB0899 initialize failed");
		return -1;
	}
}



int stb0899_set_frontend (struct dvb_frontend* fe, struct dvb_frontend_parameters* params )
{
	struct stb0899_state *state = fe->demodulator_priv;
	
	FE_STB0899_InternalParams_t *pInlParams;
	int ret = -1;
	int Retry = 2;
	
	while (Retry--)
	{
		pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
		if ( (state->m_Handle == 0) || ( (state->m_Handle != 0) && ((pInlParams->hTuner->Chip->ChipError) || (pInlParams->hDemod->ChipError))) )
		{
			st0899_initialize(fe);

			if (state->m_Handle == 0)
			{
				return -1; //initialize failed
			}
		}
		if(params != NULL)
		{
			ret = STB0899_Lock_Channel(state, params->frequency, params->u.qpsk.symbol_rate);
		}
		if (ret == 0)
		{
			return 0;
		}
		else
		{
			if (pInlParams->hTuner->Chip->ChipError || pInlParams->hDemod->ChipError)
			{
				printk(KERN_ERR "STB0899 search finish,but TunerError=%d,DemodError=%d\n", pInlParams->hTuner->Chip->ChipError, pInlParams->hDemod->ChipError);
			}
			break;
		}
	}
	return -1;
}

int stb0899_send_diseqc_msg(struct dvb_frontend* fe, struct dvb_diseqc_master_cmd* cmd)
{
	struct stb0899_state *state = fe->demodulator_priv;
	FE_STB0899_InternalParams_t *pInlParams;
	
	if (state->m_Handle)
	{
		pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
		
		ChipSetField(pInlParams->hLnb,FLNBP21_EN,1);
		if (FE_STB0899_DiseqcSend(state->m_Handle, cmd->msg, cmd->msg_len))
			return -1;
		else
			return 0;
	}
	return 0;
}

int stb0899_send_diseqc_burst(struct dvb_frontend* fe, fe_sec_mini_cmd_t minicmd)
{
	return 0;
}

int stb0899_set_tone(struct dvb_frontend* fe, fe_sec_tone_mode_t tone)
{
	struct stb0899_state *state = fe->demodulator_priv;
	FE_STB0899_InternalParams_t *pInlParams;
	
	if (state->m_Handle)
	{
		pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
		
		if (FE_STB0899_Set22KHZContinues(state->m_Handle, tone))
		{
			printk(KERN_ERR "SET_TONE FAIL\n");
			return -1;
		}
	}
	return 0;
}

int stb0899_set_voltage(struct dvb_frontend *fe, fe_sec_voltage_t voltage)
{
	struct stb0899_state *state = fe->demodulator_priv;
	FE_STB0899_InternalParams_t *pInlParams;

	if (state->m_Handle)
	{
		pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
		switch(voltage)
		{
			case SEC_VOLTAGE_13:
				ChipSetField(pInlParams->hLnb,FLNBP21_EN,1);
				ChipSetField(pInlParams->hLnb,FLNBP21_VSEL,0);
			break;	
	
			case SEC_VOLTAGE_18:
				ChipSetField(pInlParams->hLnb,FLNBP21_EN,1);
				ChipSetField(pInlParams->hLnb,FLNBP21_VSEL,1);
			break;

			case SEC_VOLTAGE_OFF:
				ChipSetField(pInlParams->hLnb,FLNBP21_EN,0);
			break;
			
			default:
				return -1;	
		}
	}
	return 0;
}

void stb0899_release(struct dvb_frontend* fe)
{
	struct stb0899_state *state = fe->demodulator_priv;
	kfree(state);

}

int stb0899_read_status(struct dvb_frontend* fe, fe_status_t* status)
{
	struct stb0899_state *state = fe->demodulator_priv;
	FE_STB0899_InternalParams_t *pInlParams;
	
	*status = 0;
	if (state->m_Handle)
	{
		pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
		switch(pInlParams->Standard)
		{
			case FE_DVBS1_STANDARD:
			case FE_DSS_STANDARD:
				state->signal = ChipGetField(pInlParams->hDemod, FSTB0899_LOCKEDVIT);
				if (state->signal)
					*status |= 0x1f;
			break;
			case FE_DVBS2_STANDARD:
				state->signal = (FE_DVBS2_GetState(pInlParams->hDemod, 10) == FE_DVBS2_DATAOK) ? 1 : 0;
				if (state->signal)
					*status |= 0x1f;
			break;
		}
	}
	else
	{
		return -1;
	}
	return 0;
}
int stb0899_read_ber(struct dvb_frontend* fe, u32* ber)
{
	struct stb0899_state *state = fe->demodulator_priv;
	FE_STB0899_InternalParams_t *pInlParams;
	
	*ber = 0;
	if (state->m_Handle)
	{
		pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
		*ber = FE_STB0899_GetError(pInlParams->hDemod, pInlParams->Standard);
	}
	else
	{
		return -1;
	}

	return 0;
}
int stb0899_read_signal_strength(struct dvb_frontend* fe, u16* strength)
{
	struct stb0899_state *state = fe->demodulator_priv;
	FE_STB0899_InternalParams_t *pInlParams;
	
	*strength = 0;
	if (state->m_Handle)
	{
		pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
		switch(pInlParams->Standard)
		{
			case FE_DVBS1_STANDARD:
			case FE_DSS_STANDARD:
				*strength = FE_STB0899_GetRFLevel(pInlParams->hDemod, &FE_STB0899_RF_LookUp, (STB0899_STANDARD)pInlParams->Standard);
			break;
			case FE_DVBS2_STANDARD:
				*strength = FE_STB0899_GetRFLevel(pInlParams->hDemod, &FE_STB0899_DVBS2RF_LookUp, (STB0899_STANDARD)pInlParams->Standard);
			break;
		}
	}
	else
	{
		return -1;
	}

	return 0;
}

int stb0899_read_snr(struct dvb_frontend* fe, u16* snr)
{
	struct stb0899_state *state = fe->demodulator_priv;
	FE_STB0899_InternalParams_t *pInlParams;
	
	*snr = 0;
	if (state->m_Handle)
	{
		pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
		
		*snr = CarrierGetQuality(pInlParams->hDemod, &FE_STB0899_CN_LookUp, (STB0899_STANDARD)pInlParams->Standard);
	}
	else
	{
		return -1;
	}

	return 0;
}

int stb0899_read_ucblocks(struct dvb_frontend* fe, u32* ucblocks)
{
	*ucblocks = 0;
	return 0;
}

static int stb0899_i2c_gate_ctrl(struct dvb_frontend* fe, int enable)
{
	struct stb0899_state * state = fe->demodulator_priv;
	FE_STB0899_InternalParams_t * pInlParams;

	pInlParams = (FE_STB0899_InternalParams_t *)state->m_Handle;
	if (enable) 
	{
		ChipSetOneRegister(pInlParams->hDemod, RSTB0899_I2CRPT, 0x5c|0x80);
	} 
	else 
	{
		ChipSetOneRegister(pInlParams->hDemod, RSTB0899_I2CRPT, 0x5c&0x7f);
	}
	return 0;
}	

 static struct dvb_frontend_ops stb0899_ops = {
	.info = {
		.name			= "ST STB0899 DVB-S2",
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
	
	.init = st0899_initialize,
	.release = stb0899_release,
	
	.set_standard = stb0899_set_standard,
	.set_frontend = stb0899_set_frontend,

	.diseqc_send_master_cmd = stb0899_send_diseqc_msg,
	.diseqc_send_burst = stb0899_send_diseqc_burst,
	.set_tone = stb0899_set_tone,
	.set_voltage = stb0899_set_voltage,

	.i2c_gate_ctrl = stb0899_i2c_gate_ctrl,
		
	.read_status = stb0899_read_status,
	.read_ber = stb0899_read_ber,
	.read_signal_strength = stb0899_read_signal_strength,
	.read_snr = stb0899_read_snr,
	.read_ucblocks = stb0899_read_ucblocks,
};

struct dvb_frontend *stb0899_attach(const struct stb0899_config *config, struct i2c_adapter *i2c_adap)
{	
	struct stb0899_state *state = NULL;

	state = kmalloc(sizeof (struct stb0899_state), GFP_KERNEL);
	if (state == NULL)
		goto error;

	state->config = config;
	state->i2c_adap = i2c_adap;
	state->g_DvbsMode = 1;
	memcpy(&state->frontend.ops, &stb0899_ops, sizeof (struct dvb_frontend_ops));
	state->frontend.demodulator_priv = state;

	return &state->frontend;
error:
	kfree(state);
	return NULL;
}

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(stb0899_attach);

