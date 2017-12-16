
#include "stb0899_dvbs2util.h"
#include "stb0899_init.h"
#include "stb0899_util.h"


/*Set carrier freq (mhz) masterclock mhz*/
static void FE_DVBS2_SetCarrierFreq(STCHIP_Handle_t hChip, S32 CarrierFreq, u32 MasterClock)
{
	S32 crlNomFreq;
	
	crlNomFreq=(PowOf2(CRL_NCO_BITS))/MasterClock;
	crlNomFreq*=CarrierFreq;
	ChipSetField(hChip,FSTB0899_CRLNOM_FREQ,crlNomFreq);
}

static u32 DVBS2CalclSymbRate(u32 SymbolRate, u32 MasterClock)
{
	u32	decimRatio,
		decimRate,
		decimation,
		remain,
		intval,
		btrNomFreq;
	
	decimRatio = (MasterClock*2) / (5 * SymbolRate);
	decimRatio = (decimRatio == 0) ? 1 : decimRatio;
	decimRate = Log2Int(decimRatio);
	decimation = 1 << decimRate;
	MasterClock /= 1000; /* for integer Caculation*/
	SymbolRate /= 1000;  /* for integer Caculation*/
	
	if(decimation <= 4)
    	{
    		intval = (decimation*PowOf2(BTR_NCO_BITS-1)) / MasterClock;
     		remain = (decimation*PowOf2(BTR_NCO_BITS-1)) % MasterClock;
    	}
	else
    	{
    		intval = PowOf2(BTR_NCO_BITS-1) / (MasterClock/100) * decimation / 100;
     		remain = (decimation * PowOf2(BTR_NCO_BITS-1)) % MasterClock;
	}
	btrNomFreq =(intval*SymbolRate) + ((remain*SymbolRate) / MasterClock);
	
	return btrNomFreq;
}

static u32 CalcCorrection(u32 SymbolRate, u32 MasterClock)
{
	u32 decimRatio, correction;
	
	decimRatio = (MasterClock*2) / (5 * SymbolRate);
	decimRatio = (decimRatio == 0) ? 1 : decimRatio;

	MasterClock/=1000;	/* for integer Caculation*/
	SymbolRate/=1000;	/* for integer Caculation*/
	correction=(512 * MasterClock) / (2 * decimRatio * SymbolRate);

	return correction;
}
	
/* Sets the symbol rate to demodulate  (valid range 1.0 MHz - 45.0 MHz */
static void FE_DVBS2_SetSymbolRate(STCHIP_Handle_t hChip, u32 SymbolRate, u32 MasterClock)
{
 	u32	decimRatio,
 		decimRate,
 		winSel,
 		decimation,
 		fSymovSr,
 		btrNomFreq,
 		correction,
 		freqAdjScl,
 		bandLimit,
 		decimcntrlreg;
 
 	/*set decimation to 1*/
	decimRatio = (MasterClock*2) / (5 * SymbolRate);
	decimRatio = (decimRatio == 0) ? 1 : decimRatio;
	decimRate = Log2Int(decimRatio);

	winSel=0;
	if(decimRate >= 5)
		winSel = decimRate - 4;
	
	decimation = (1 << decimRate);

	/* (FSamp/Fsymbol *100) for integer Caculation */
	fSymovSr=MasterClock / ((decimation*SymbolRate) / 1000);
	
	/* don't band limit signal going into btr block*/
	if(fSymovSr<=2250)	
		bandLimit=1;
 	else
    		bandLimit=0;	/* band limit signal going into btr block*/

    	decimcntrlreg=((winSel << 3) & 0x18) + ((bandLimit << 5) & 0x20) + (decimRate & 0x7);
	ChipSetOneRegister(hChip, RSTB0899_DECIMCNTRL, decimcntrlreg);

	if(fSymovSr<=3450)
		ChipSetOneRegister(hChip, RSTB0899_ANTIALIASSEL, 0);
	else if(fSymovSr<=4250)
    		ChipSetOneRegister(hChip, RSTB0899_ANTIALIASSEL, 1);
    	else
    		ChipSetOneRegister(hChip, RSTB0899_ANTIALIASSEL, 2);

	btrNomFreq = DVBS2CalclSymbRate(SymbolRate, MasterClock);
	ChipSetOneRegister(hChip, RSTB0899_BTRNOMFREQ, btrNomFreq);

	correction = CalcCorrection(SymbolRate, MasterClock);
	ChipSetField(hChip, FSTB0899_BTRFREQ_CORR, correction);

	/* scale UWP+CSM frequency to sample rate*/
	freqAdjScl = SymbolRate / (MasterClock / 4096);
	ChipSetOneRegister(hChip, RSTB0899_FREQADJSCALE, freqAdjScl);
}

/* Sets the Bit Timing loop bandwidth as a percentage of the symbol rate */
static void FE_DVBS2_SetBtrLoopBW(STCHIP_Handle_t hChip, FE_DVBS2_LoopBW_Params_t LoopBW)
{
	S32 	decimRatio,
    		decimRate,
    		kbtr1Rshft,
    		kbtr1,
    		kbtr0Rshft,
    		kbtr0,
    		kbtr2Rshft,
    		kDirectShift,
    		kIndirectShift;
	
	u32	decimation,
		K,
		wn,
		kDirect,
		kIndirect;

	decimRatio = (LoopBW.MasterClock * 2) / (5 * LoopBW.SymbolRate);
	decimRatio = (decimRatio == 0) ? 1 : decimRatio;
	decimRate = Log2Int(decimRatio);
	decimation = (1 << decimRate);

	LoopBW.SymPeakVal = LoopBW.SymPeakVal * 576000;
   
	K = PowOf2(BTR_NCO_BITS) / (LoopBW.MasterClock / 1000);
	K *= (LoopBW.SymbolRate / 1000000) * decimation; /*k=k 10^-8*/
   
	K = LoopBW.SymPeakVal / K;

	if(K != 0)
	{
		/*wn =wn 10^-8*/
		wn = (4 * LoopBW.Zeta * LoopBW.Zeta) + 1000000;
		wn =(2 * (LoopBW.LoopBwPercent * 1000) * 40 * LoopBW.Zeta) / wn;
	
		kIndirect = (wn * wn) / K;
		kIndirect = kIndirect;	/*kindirect = kindirect 10^-6*/
	
		kDirect = (2 * wn * LoopBW.Zeta) / K;	/*kDirect = kDirect 10^-2*/
		kDirect *= 100;
	
		kDirectShift = Log2Int(kDirect) - Log2Int(10000) - 2;
		kbtr1Rshft = (-1 * kDirectShift) + BTR_GAIN_SHIFT_OFFSET;
		kbtr1 = kDirect / PowOf2(kDirectShift);
		kbtr1 /= 10000;

		kIndirectShift = Log2Int(kIndirect + 15) - 20;
		kbtr0Rshft = (-1 * kIndirectShift) + BTR_GAIN_SHIFT_OFFSET;
		kbtr0 = kIndirect * PowOf2(-kIndirectShift);
		kbtr0 /= 1000000;

		kbtr2Rshft = 0;
		if( kbtr0Rshft > 15)
		{
			kbtr2Rshft = kbtr0Rshft - 15;
			kbtr0Rshft = 15;
		}
		
		ChipSetFieldImage(hChip, FSTB0899_KBTR0_RSHFT, kbtr0Rshft);
		ChipSetFieldImage(hChip, FSTB0899_KBTR0, kbtr0);
		ChipSetFieldImage(hChip, FSTB0899_KBTR1_RSHT, kbtr1Rshft);
		ChipSetFieldImage(hChip, FSTB0899_KBTR1, kbtr1);
		ChipSetFieldImage(hChip, FSTB0899_KBTR2_RSHT, kbtr2Rshft);
	
		ChipSetRegisters(hChip, RSTB0899_BTRLOOPGAIN, 1);
	}
	else
		ChipSetOneRegister(hChip, RSTB0899_BTRLOOPGAIN, 0xc4c4f);
}

/* Initializes the BTR loop hardware */
static void FE_DVBS2_BtrInit(STCHIP_Handle_t hChip)
{	
	/* set enable BTR loopback*/
	ChipSetFieldImage(hChip, FSTB0899_INTRP_PHS_SENS, 1);
	ChipSetFieldImage(hChip, FSTB0899_BTRERR_ENA, 1);
	ChipSetRegisters(hChip, RSTB0899_BTRCNTRL, 1);

	/* fix btr freq accum at 0*/
	ChipSetOneRegister(hChip, RSTB0899_BTRFREQINIT, 0x10000000);
	ChipSetOneRegister(hChip, RSTB0899_BTRFREQINIT, 0x00000000);
	
	/* fix btr freq accum at 0*/
	ChipSetOneRegister(hChip, RSTB0899_BTRPHSINIT, 0x10000000);
	ChipSetOneRegister(hChip, RSTB0899_BTRPHSINIT, 0x00000000);
}

static void FE_DVBS2_Reset(STCHIP_Handle_t hChip)
{
	u32 	imbHold,
		dcHold;
	
	int 	i=0;
 
	ChipSetFieldImage(hChip, FSTB0899_IF_LDGAININIT, 1);
	ChipSetRegisters(hChip, RSTB0899_IFAGCCNTRL, 1);
	
	ChipSetFieldImage(hChip, FSTB0899_BBLDGAIN_INIT, 1);
	ChipSetRegisters(hChip, RSTB0899_BBAGCCNTRL, 1);
	
	ChipSetFieldImage(hChip, FSTB0899_CRLPHSINIT31, 1);
	ChipSetRegisters(hChip, RSTB0899_CRLPHSINIT, 1);
	
	ChipSetFieldImage(hChip, FSTB0899_CRLFREQINIT31, 1);
	ChipSetRegisters(hChip, RSTB0899_CRLFREQINIT, 1);
	
  	ChipSetFieldImage(hChip, FSTB0899_BTRID_PHASEINIT, 1);
  	ChipSetRegisters(hChip, RSTB0899_BTRPHSINIT, 1);
	
  	ChipSetFieldImage(hChip, FSTB0899_BTRID_FREQINIT, 1);
  	ChipSetRegisters(hChip, RSTB0899_BTRFREQINIT, 1);
	
	imbHold=ChipGetOneRegister(hChip, RSTB0899_IMBCNTRL);
	
	ChipSetFieldImage(hChip, FSTB0899_IMB_COMP, 0);
	ChipSetRegisters(hChip, RSTB0899_IMBCNTRL, 1);
	
	dcHold=ChipGetOneRegister(hChip, RSTB0899_DMDCNTRL);
	
	ChipSetFieldImage(hChip, FSTB0899_DC_COMP, 0);
	ChipSetRegisters(hChip, RSTB0899_DMDCNTRL, 1);
	
	ChipSetFieldImage(hChip, FSTB0899_CRL_CLR_PHSERR, 1);
	ChipSetRegisters(hChip, RSTB0899_CRLCNTRL, 1);
	
	/*WAIT_N_MS(2);*/
	ChipSetFieldImage(hChip, FSTB0899_IF_LDGAININIT, 0);
	ChipSetRegisters(hChip, RSTB0899_IFAGCCNTRL, 1);
			
	/*whait for IF AGC lock*/
	while((ChipGetField(hChip, FSTB0899_IF_AGCLOCK) != 1) && (i < 30))
		i++;

	ChipSetFieldImage(hChip, FSTB0899_BTRID_PHASEINIT, 0);
	ChipSetRegisters(hChip, RSTB0899_BTRPHSINIT, 1);

	/*WAIT_N_MS(2);*/
	ChipSetFieldImage(hChip, FSTB0899_CRLPHSINIT31, 0);
	ChipSetRegisters(hChip, RSTB0899_CRLPHSINIT, 1);

	ChipSetFieldImage(hChip, FSTB0899_CRLFREQINIT31, 0);
	ChipSetRegisters(hChip, RSTB0899_CRLFREQINIT, 1);

   	ChipSetFieldImage(hChip, FSTB0899_BBLDGAIN_INIT, 0);
    	ChipSetRegisters(hChip, RSTB0899_BBAGCCNTRL, 1);

    	ChipSetFieldImage(hChip, FSTB0899_BTRID_FREQINIT, 0);
   	ChipSetRegisters(hChip, RSTB0899_BTRFREQINIT, 1);

    	ChipSetOneRegister(hChip, RSTB0899_IMBCNTRL,imbHold | 1);
	ChipSetOneRegister(hChip, RSTB0899_DMDCNTRL,dcHold | 14);
	
	ChipSetFieldImage(hChip, FSTB0899_CRL_CLR_PHSERR, 0);
	ChipSetRegisters(hChip, RSTB0899_CRLCNTRL, 1);
}

/* config UWP */
static void FE_DVBS2_ConfigUWP(STCHIP_Handle_t hChip, FE_DVBS2_UWPConfig_Params_t UWPparams)
{
	/*Set Fields imgae value*/
	ChipSetFieldImage(hChip, FSTB0899_UWP_ESN0_AVE, UWPparams.EsNoAve);
	ChipSetFieldImage(hChip, FSTB0899_UWP_ESN0_QUANT, UWPparams.EsNoQuant);
	ChipSetFieldImage(hChip, FSTB0899_UWP_THRESHOLD_SOF, UWPparams.ThresholdSof);
	ChipSetFieldImage(hChip, FSTB0899_FE_COARSE_TRK, UWPparams.AveFramesCoarse);
	ChipSetFieldImage(hChip, FSTB0899_FE_FINE_TRK, UWPparams.AveframesFine);
	ChipSetFieldImage(hChip, FSTB0899_UWP_MISS_THRESHOLD, UWPparams.MissThreshold);
	ChipSetFieldImage(hChip, FSTB0899_UWP_THRESHOLD_ACQ, UWPparams.ThresholdAcq);
	ChipSetFieldImage(hChip, FSTB0899_UWP_THRESHOLD_TRACK, UWPparams.ThresholdTrack);
	
	/*write values to registers*/
	ChipSetRegisters(hChip, RSTB0899_UWPCNTRL1, 1);
	ChipSetRegisters(hChip, RSTB0899_UWPCNTRL2, 1);
	ChipSetRegisters(hChip, RSTB0899_UWPCNTRL3, 1);
	
	ChipSetOneRegister(hChip, RSTB0899_SOFSRCHTO, UWPparams.SofSearchTimeout);
}

/* start UWP */
static void FE_DVBS2_StartUWP(STCHIP_Handle_t hChip)
{
	/* write a 1 to the start reg */
	ChipSetFieldImage(hChip, FSTB0899_UWP_START, 1);
	ChipSetRegisters(hChip, RSTB0899_UWPCNTRL1, 1);
	
	/* write a 0 to the start reg */
	ChipSetField(hChip, FSTB0899_UWP_START, 0);
	ChipSetRegisters(hChip, RSTB0899_UWPCNTRL1, 1);
}

/* config CSM with internal stored parameters */
static void FE_DVBS2_AutoConfigCSM(STCHIP_Handle_t hChip)
{
	/* to auto=config write a 1 to auto_param register */
	/*Set filed image value*/
	ChipSetFieldImage(hChip, FSTB0899_AUTO_PARAM, 1);
	/*write value to the register*/
	ChipSetRegisters(hChip, RSTB0899_CSMCNTRL1, 1);
}

static void FE_DVBS2_ManualConfigCSM(STCHIP_Handle_t hChip,FE_DVBS2_CSMConfig_Params_t CSMParams)
{
	/* to manually config write a 0 to auto_param register*/

	/*Set filed image value*/
	ChipSetFieldImage(hChip, FSTB0899_AUTO_PARAM, 0);
	/*write value to the register*/
	ChipSetRegisters(hChip, RSTB0899_CSMCNTRL1, 1);
	
	/*configure other registers*/
	ChipSetFieldImage(hChip, FSTB0899_CSM_DVT_TABLE, CSMParams.DvtTable);
	ChipSetFieldImage(hChip, FSTB0899_CSM_TOW_PASS, CSMParams.TwoPass);
	ChipSetFieldImage(hChip, FSTB0899_CSM_AGC_GAIN, CSMParams.AgcGain);
	ChipSetFieldImage(hChip, FSTB0899_CSM_AGC_SHIFT, CSMParams.AgcShift);
	ChipSetFieldImage(hChip, FSTB0899_FE_LOOP_SHIFT, CSMParams.FeLoopShift);
	ChipSetFieldImage(hChip, FSTB0899_CSM_GAMMA_ACQ, CSMParams.GammaAcq);
	ChipSetFieldImage(hChip, FSTB0899_CSM_GAMMA_RHOACQ, CSMParams.GammaRhoAcq);
	ChipSetFieldImage(hChip, FSTB0899_CSM_GAMMA_TRACK, CSMParams.GammaTrack);
	ChipSetFieldImage(hChip, FSTB0899_CSM_GAMMA_RHOTRACK, CSMParams.GammaRhoTrack);
	ChipSetFieldImage(hChip, FSTB0899_LOCK_COUNT_THRESHOLD, CSMParams.LockCountThreshold);
	ChipSetFieldImage(hChip, FSTB0899_PHASE_DIFF_THRESHOLD, CSMParams.PhaseDiffThreshold);
	
	ChipSetRegisters(hChip, RSTB0899_CSMCNTRL1, 1);
	ChipSetRegisters(hChip, RSTB0899_CSMCNTRL2, 1);
	ChipSetRegisters(hChip, RSTB0899_CSMCNTRL3, 1);
	ChipSetRegisters(hChip, RSTB0899_CSMCNTRL4, 1);
}

/* for future regarding QPSK w/Pilots over sampling <=4*/
void FE_DVBS2_CSMInitialize(STCHIP_Handle_t hChip, int Pilots, FE_DVBS2_ModCod_t ModCode, u32 SymbolRate, u32 MasterClock)
{
	FE_DVBS2_CSMConfig_Params_t csmParams;
	
	csmParams.DvtTable = 1;
	csmParams.TwoPass = 0;
	csmParams.AgcGain = 6;
	csmParams.AgcShift = 0;
	csmParams.FeLoopShift = 0;
	csmParams.PhaseDiffThreshold = 0x80;
	
	if( ((MasterClock / SymbolRate) <= 4) && (ModCode <= 11) && (Pilots == 1))
	{
		switch (ModCode)
		{
			case FE_QPSK_12:
				csmParams.GammaAcq = 25;
				csmParams.GammaRhoAcq = 2700;
				csmParams.GammaTrack = 12;
				csmParams.GammaRhoTrack = 180;
				csmParams.LockCountThreshold = 8;
			break;
			
			case FE_QPSK_35:
				csmParams.GammaAcq = 38;
				csmParams.GammaRhoAcq = 7182;
				csmParams.GammaTrack = 14;
				csmParams.GammaRhoTrack = 308;
				csmParams.LockCountThreshold = 8;
			break;
			
			case FE_QPSK_23:
				csmParams.GammaAcq = 42;
				csmParams.GammaRhoAcq = 9408;
				csmParams.GammaTrack = 17;
				csmParams.GammaRhoTrack = 476;
				csmParams.LockCountThreshold = 8;
			break;
			
			case FE_QPSK_34:
				csmParams.GammaAcq = 53;
				csmParams.GammaRhoAcq = 16642;
				csmParams.GammaTrack = 19;
				csmParams.GammaRhoTrack = 646;
				csmParams.LockCountThreshold = 8;
			break;
			
			case FE_QPSK_45:
				csmParams.GammaAcq = 53;
				csmParams.GammaRhoAcq = 17119;
				csmParams.GammaTrack = 22;
				csmParams.GammaRhoTrack = 880;
				csmParams.LockCountThreshold = 8;
			break;
			
			case FE_QPSK_56:
				csmParams.GammaAcq = 55;
				csmParams.GammaRhoAcq = 19250;
				csmParams.GammaTrack = 23;
				csmParams.GammaRhoTrack = 989;
				csmParams.LockCountThreshold = 8;
			break;
			
			case FE_QPSK_89:
				csmParams.GammaAcq = 60;
				csmParams.GammaRhoAcq = 24240;
				csmParams.GammaTrack = 24;
				csmParams.GammaRhoTrack = 1176;
				csmParams.LockCountThreshold = 8;
			break;
			
			case FE_QPSK_910:
				csmParams.GammaAcq = 66;
				csmParams.GammaRhoAcq = 29634;
				csmParams.GammaTrack = 24;
				csmParams.GammaRhoTrack = 1176;
				csmParams.LockCountThreshold = 8;
			break;
			
			default:
				csmParams.GammaAcq = 66;
				csmParams.GammaRhoAcq = 29634;
				csmParams.GammaTrack = 24;
				csmParams.GammaRhoTrack = 1176;
				csmParams.LockCountThreshold = 8;
			break;
		}
		FE_DVBS2_ManualConfigCSM(hChip, csmParams);
	}	
}

/* read csm lock */
static int FE_DVBS2_GetCSMLock(STCHIP_Handle_t hChip, int TimeOut)
{
	int 	Time=0,
		CSMLocked=0;

	while((!CSMLocked) && (Time < TimeOut))
	{
		CSMLocked = ChipGetField(hChip, FSTB0899_CSM_LOCK);
		Time++;
	}
	
	return CSMLocked;
}

/* read uwp_state */
static int FE_DVBS2_GetUWPstate(STCHIP_Handle_t hChip, int TimeOut)
{
	int 	Time=0,
		lock=0,
		locked=0;
	
	while((Time < TimeOut) && (lock == 0))
	{
		lock = ChipGetField(hChip, FSTB0899_UWP_LOCK);
		Time++;
	}
	if(lock)
		locked = 1;

	return locked;
}

static int FE_DVBS2_GetDataLock(STCHIP_Handle_t hChip, int TimeOut)
{
	int 	Time = 0,
	  	DataLocked = 0;

	while((!DataLocked) && (Time < TimeOut))
	{
		DataLocked = ChipGetField(hChip, FSTB0899_LOCK);
		Time++;
	}
	
	return DataLocked; 
}

FE_DVBS2_State FE_DVBS2_GetState(STCHIP_Handle_t hChip,int Timeout)
{
	int tout, i = 0;
	FE_DVBS2_State state = FE_DVBS2_NOAGC;

	tout = Timeout;
	do
	{
		if(FE_DVBS2_GetCSMLock(hChip, 1) != TRUE)
			state = FE_DVBS2_NOCARRIER;
		else 
			if((FE_DVBS2_GetUWPstate(hChip, 1) != TRUE))
				state = FE_DVBS2_NOUWP;
		else
			if(FE_DVBS2_GetDataLock(hChip, 1) != TRUE)
				state = FE_DVBS2_NODATA;
		else	
			state = FE_DVBS2_DATAOK;

		WAIT_N_MS(1);
		i++;
	}while((i < tout) && (state != FE_DVBS2_DATAOK));

	return state;
}

/* read modecode */
u32 FE_DVBS2_GetModCod(STCHIP_Handle_t hChip)
{
	return(ChipGetField(hChip,FSTB0899_UWP_DECODED_MODCODE) >> 2);
}


/* read EsNo */
S32 FE_DVBS2_GetUWPEsNo(STCHIP_Handle_t hChip,S32 Quant)
{
	u32	tempus;
	
	tempus = ChipGetField(hChip,FSTB0899_ESN0_ESR);
	return tempus; /* to convert value to db tempus=(10*log10(tempus)/(quant^2)*/
	/* with quant = UWP_ESN0_QUANT field value  */
}

/*****************************************************
--FUNCTION	::	FE_DVBS2_CarrierWidth
--ACTION	::	Compute the width of the carrier
--PARAMS IN	::	SymbolRate->Symbol rate of the carrier (Kbauds or Mbauds)
--			RollOff	->Rolloff * 100
--PARAMS OUT	::	NONE
--RETURN	::	Width of the carrier (KHz or MHz) 
--***************************************************/
long FE_DVBS2_CarrierWidth(long SymbolRate, FE_DVBS2_RRCAlpha_t Alpha)
{
	long RollOff = 0;
	
	switch(Alpha)
	{
		case RRC_20:
			RollOff = 20;
		break;
		case RRC_25:
			RollOff = 25;
		break;
		case RRC_35:
			RollOff = 35;
		break;
	}
	return (SymbolRate + (SymbolRate * RollOff) / 100);
}

u32 FE_DVBS2_GetSymbolRate(STCHIP_Handle_t hChip,u32 MasterClock)
{
	u32	bTrNomFreq,
		symbolRate,
		decimRate,
		intval1,intval2;
	int div1, div2, rem1, rem2;

	div1 = BTR_NCO_BITS/2;
	div2 = BTR_NCO_BITS-div1-1;
	
	bTrNomFreq = ChipGetOneRegister(hChip,RSTB0899_BTRNOMFREQ);
	decimRate = ChipGetField(hChip,FSTB0899_DECIM_RATE);
	decimRate = PowOf2(decimRate);

	intval1 = MasterClock/PowOf2(div1);
	intval2 = bTrNomFreq/PowOf2(div2);

	/*symbrate = (btrnomfreq_register_val*MasterClock)/2^(27+decim_rate_field) */
	rem1 = MasterClock%PowOf2(div1);
	rem2 = bTrNomFreq%PowOf2(div2);
	symbolRate = (intval1*intval2)+((intval1*rem2)/PowOf2(div2))+((intval2*rem1)/PowOf2(div1));
	symbolRate /= decimRate;

	return symbolRate;
}

void FE_DVBS2_InitialCalculations(STCHIP_Handle_t hChip, FE_STB0899_DVBS2_InitParams_t *InitParams)
{
	FE_DVBS2_UWPConfig_Params_t uwpParams;
	FE_DVBS2_LoopBW_Params_t loopBW;
	
	uwpParams.EsNoAve = ESNO_AVE;
	uwpParams.EsNoQuant = ESNO_QUANT;
	uwpParams.AveFramesCoarse = InitParams->AveFrameCoarse;
	uwpParams.AveframesFine = InitParams->AveFramefine;
	uwpParams.MissThreshold = MISS_THRESHOLD;
	uwpParams.ThresholdAcq = UWP_THRESHOLD_ACQ;
	uwpParams.ThresholdTrack = UWP_THRESHOLD_TRACK;
	uwpParams.ThresholdSof = UWP_THRESHOLD_SOF;
	uwpParams.SofSearchTimeout = SOF_SEARCH_TIMEOUT;
	
	loopBW.LoopBwPercent = 60;
	loopBW.SymbolRate = InitParams->SymbolRate;
	loopBW.MasterClock = InitParams->MasterClock;
	loopBW.Mode = InitParams->ModeMode;
	loopBW.Zeta = 707;
	loopBW.SymPeakVal = InitParams->AgcThreshold;/**5.76*/
	
	/* config uwp and csm */ 
	FE_DVBS2_ConfigUWP(hChip, uwpParams);
	FE_DVBS2_AutoConfigCSM(hChip);

	/* initialize BTR	*/
	FE_DVBS2_SetSymbolRate(hChip, InitParams->SymbolRate, InitParams->MasterClock);
	FE_DVBS2_SetBtrLoopBW(hChip, loopBW);
	FE_DVBS2_BtrInit(hChip);
	
	/* enable frequency adjustment and set spectral inversion */
	ChipSetOneRegister(hChip, RSTB0899_DMDCNTRL2, (2 | (InitParams->SpectralInv << 2)));
	
	/* disable CRL	*/
	ChipSetOneRegister(hChip, RSTB0899_CRLFREQINIT, 1 << 30);
	ChipSetOneRegister(hChip, RSTB0899_CRLLOOPGAIN, 0);
	ChipSetOneRegister(hChip, RSTB0899_CRLPHSINIT, 1 << 30);
	ChipSetOneRegister(hChip, RSTB0899_CRLPHSINIT, 0);
}

void FE_DVBS2_Reacquire(STCHIP_Handle_t hChip, FE_DVBS2_ReacquireParams_t * ReacquireParams)
{
	S32 	numSteps,
		freqStepSize,
		acqcntrl2;
				
	/*disable input buff*/
	ChipSetOneRegister(hChip, RSTB0899_INTBUFCTRL, 0x00);
	
	/* set demod soft reset*/
	ChipSetOneRegister(hChip, RSTB0899_RESETCNTRL, 0x1);
	
	FE_DVBS2_Reset(hChip);
	/*release demod soft reset */
	
	ChipSetOneRegister(hChip, RSTB0899_RESETCNTRL, 0);
	
	/* reset decoder */
	ChipSetOneRegister(hChip, RSTB0899_LDPCDECRST, 1);
	ChipSetOneRegister(hChip, RSTB0899_LDPCDECRST, 0);

	if(ReacquireParams->AcqMode == NO_SEARCH)
	{
    		FE_DVBS2_StartUWP(hChip);
    	}
    	else
    	{
	   	 /*numSteps =(ReacquireParams->FreqRange*100)/((ReacquireParams->SymbolRate/1000000)*ReacquireParams->StepSize);
	   	 numSteps=(numSteps+5)/10;
	   	 freqStepSize =(ReacquireParams->StepSize * (1<<17))/10;*/

		if(ReacquireParams->SymbolRate / 1000000 >= 15)
			freqStepSize = (1 << 17) / 5;
		else if(ReacquireParams->SymbolRate/ 1000000 >= 10)
			freqStepSize = (1 << 17) / 7;
		
		else if(ReacquireParams->SymbolRate / 1000000 >= 5)
			freqStepSize = (1 << 17) / 10;
		else
			freqStepSize = (1 << 17) / 3;

		numSteps = (10 * ReacquireParams->FreqRange * (1 << 17)) / (freqStepSize * (ReacquireParams->SymbolRate / 1000000));
		numSteps = (numSteps + 6) / 10;
	   	numSteps = (numSteps == 0) ? 1 : numSteps;

		if(ReacquireParams->Zigzag)
	    		if(numSteps % 2 == 0)
				FE_DVBS2_SetCarrierFreq(hChip, ReacquireParams->CenterFreq - (ReacquireParams->StepSize * (ReacquireParams->SymbolRate / 20000000)), (ReacquireParams->MasterClock) / 1000000);
	    		else
				FE_DVBS2_SetCarrierFreq(hChip, ReacquireParams->CenterFreq, (ReacquireParams->MasterClock) / 1000000);
	   	 else
	      		FE_DVBS2_SetCarrierFreq(hChip, ReacquireParams->CenterFreq - (ReacquireParams->FreqRange / 2),(ReacquireParams->MasterClock) / 1000000);
		
		/*Set Carrier Search params (zigzag, num steps and freq step size*/
		acqcntrl2 = (ReacquireParams->Zigzag << 25) | (numSteps << 17) | (freqStepSize);
   	    	ChipSetOneRegister(hChip, RSTB0899_ACQCNTRL2, acqcntrl2);

		/* Equalizer Init  */
		ChipSetFieldImage(hChip, FSTB0899_EQ_INIT, 1);
		ChipSetRegisters(hChip, RSTB0899_EQUILIZERINIT, 1);
		ChipSetFieldImage(hChip, FSTB0899_EQ_INIT, 0);
		ChipSetRegisters(hChip, RSTB0899_EQUILIZERINIT, 1);
		
		/* Equalizer Disable update  */ 
		ChipSetFieldImage(hChip, FSTB0899_EQ_DISABLE_UPDATE, 0);
		ChipSetRegisters(hChip, RSTB0899_EQCNTL, 1);

		/* start acquistion process  */
		ChipSetOneRegister(hChip, RSTB0899_ACQUIRETRIG, 1);
		ChipSetOneRegister(hChip, RSTB0899_LOCKLOST, 0);
		
		/*activate input buff*/
		ChipSetOneRegister(hChip, RSTB0899_INTBUFCTRL, 0x0a);

		/*Reset Packet Delin*/
		ChipSetField(hChip, FSTB0899_ALGOSWRST, 1);
		ChipSetField(hChip, FSTB0899_ALGOSWRST, 0);
		ChipSetField(hChip, FSTB0899_HYSTSWRST, 1);
		ChipSetField(hChip, FSTB0899_HYSTSWRST, 0);
		ChipSetOneRegister(hChip, RSTB0899_PDELCTRL, 0x4a);
		
		/*Reset stream merger*/
		ChipSetField(hChip, FSTB0899_FRESRS, 1);
		ChipSetField(hChip, FSTB0899_FRESRS, 0);
	}
}



