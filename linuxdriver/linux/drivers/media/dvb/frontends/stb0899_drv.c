/* -------------------------------------------------------------------------
File Name: STB0899_drv.c

Description: STB0899 driver LLA	V3.4 December/05/2005 


author: BJ
---------------------------------------------------------------------------- */


/* includes ---------------------------------------------------------------- */

#include <linux/dvb/frontend.h>
#include "stb0899_drv.h"

 FE_STB0899_LOOKUP_t FE_STB0899_CN_LookUp = {
	20,{
	{15,9600},{20,9450},{30,9000},{40,8250},{50,7970},{60,7360},{70,6770},{80,6200},
	{90,5670},{100,	5190},{110,4740},{120,4360},{130,4010},{140,3710},{150,3440},
	{160,3210},{170,3020},{180,2860},{190,2700},{200,2600}
	}
};
	
 FE_STB0899_LOOKUP_t FE_STB0899_RF_LookUp = {
	20,{
	{-5,79},{-10,73},{-15,67},{-20,62},{-25,56},{-30,49},{-33,44},{-35,40},{-37,36},
	{-38,33},{-40,27},{-45,4},{-47,-11},{-48,-19},{-50,-29},{-55,-43},{-60,-52},
	{-65,-61},{-67,-65},{-70,-128},
	}
};

 FE_STB0899_LOOKUP_t FE_STB0899_DVBS2RF_LookUp = {
	15,{
	{-5,2899},{-10, 3330},{-15, 3123},{-20, 3577},{-25, 4004},{-30, 4417},{-35, 4841},
	{-40, 5300},{-45, 5822},{-50, 6491},{-55, 7516},{-60, 9235},{-65, 11374},
	{-70, 12364},{-75, 13063},
	}
};

/*****************************************************
**FUNCTION	::	FE_STB0899_SetInternalError
**ACTION		::	Set the internal error value and location 
**PARAMS IN	::	Type	==> Type of the error
**			Location==> Location of the error
**PARAMS OUT	::	pError
**RETURN	::	NONE
*****************************************************/
static void FE_STB0899_SetInternalError(FE_STB0899_ErrorType_t Type,FE_STB0899_Location_t Location,FE_STB0899_InternalError_t *pError)
{
	if(pError != NULL)
	{
		pError->Type = Type;
		pError->Location = Location;
	}
}

/*****************************************************
**FUNCTION	::	WaitTuner
**ACTION	::	Wait for tuner locked
**PARAMS IN	::	TimeOut	->Maximum waiting time (in ms) 
**PARAMS OUT::	NONE
**RETURN	::	NONE
*****************************************************/
static void WaitTuner(TUNER_Handle_t hTuner,int TimeOut)
{
	int Time=0;
	int TunerLocked = FALSE;
	
	while(!TunerLocked && (Time<TimeOut))
	{
		WAIT_N_MS(1);
		TunerLocked = TunerGetStatus(hTuner);
		
		Time++;
	}
	Time--;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_CheckTiming
--ACTION	::	Check for timing locked
--PARAMS IN	::	pParams->Ttiming=>Time to wait for timing loop locked
--PARAMS OUT	::	pParams->State	=>result of the check
--RETURN	::	NOTIMING if timing not locked, TIMINGOK otherwise
--***************************************************/
static FE_STB0899_SIGNALTYPE_t FE_STB0899_CheckTiming(FE_STB0899_InternalParams_t *pParams)
{
	int locked,timing;
	
	WAIT_N_MS(pParams->Ttiming);
	ChipSetField(pParams->hDemod,FSTB0899_TIMING_LOOP_FREQ,0xf2);
	locked=ChipGetField(pParams->hDemod,FSTB0899_TMG_LOCK_IND);
	timing=ABS(ChipGetField(pParams->hDemod,FSTB0899_TIMING_LOOP_FREQ));
	if(locked >= 42)
	{
		if((locked > 48) && (timing >= 110))
			pParams->State = ANALOGCARRIER;
		else 
			pParams->State = TIMINGOK;
	}
	else
		pParams->State = NOTIMING;

	return pParams->State;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_CheckCarrier
--ACTION	::	Check for carrier founded
--PARAMS IN	::	pParams	=>Pointer to FE_STB0899_InternalParams_t structure
--PARAMS OUT	::	pParams->State	=> Result of the check
--RETURN	::	NOCARRIER carrier not founded, CARRIEROK otherwise
--***************************************************/
static FE_STB0899_SIGNALTYPE_t FE_STB0899_CheckCarrier(FE_STB0899_InternalParams_t *pParams)
{
	WAIT_N_MS(pParams->Tderot); /*wait for derotator ok*/
	ChipSetField(pParams->hDemod,FSTB0899_CFD_ON,0);
	
	if (ChipGetField(pParams->hDemod,FSTB0899_CARRIER_FOUND/*FSTB0899_FDCT*/))
		pParams->State = CARRIEROK;
	else
		pParams->State = NOCARRIER;
	
	return pParams->State;
}

static u32 FE_STB0899_GetErrorCount(STCHIP_Handle_t hChip,ERRORCOUNTER Counter)
{
	u32 lsb=0,msb=0;
	
	/*Do not modified the read order (lsb first)*/
	switch(Counter)
	{
		case COUNTER1:
			lsb = ChipGetField(hChip,FSTB0899_ERROR_COUNT_LSB);
			msb = ChipGetField(hChip,FSTB0899_ERROR_COUNT_MSB);
		break;
	
		case COUNTER2:
			lsb = ChipGetField(hChip,FSTB0899_ERROR_COUNT2_LSB);
			msb = ChipGetField(hChip,FSTB0899_ERROR_COUNT2_MSB);
		break;
		
		case COUNTER3:
			lsb = ChipGetField(hChip,FSTB0899_ERROR_COUNT3_LSB);
			msb = ChipGetField(hChip,FSTB0899_ERROR_COUNT3_MSB);
		break;
	}

	return (MAKEWORD(msb,lsb));
}

/*****************************************************
--FUNCTION	::	FE_STB0899_CheckData
--ACTION	::	Check for data founded
--PARAMS IN	::	pParams	=>Pointer to FE_STB0899_InternalParams_t structure
--PARAMS OUT	::	pParams->State	=> Result of the check
--RETURN	::	NODATA data not founded, DATAOK otherwise
--***************************************************/
static FE_STB0899_SIGNALTYPE_t FE_STB0899_CheckData(FE_STB0899_InternalParams_t *pParams)
{
	int lock = 0, index=0, dataTime=500;
	pParams->State = NODATA;
	
	/* reset du FEC */
	ChipSetField(pParams->hDemod,FSTB0899_FRESACS,1);
	WAIT_N_MS(1);
	ChipSetField(pParams->hDemod,FSTB0899_FRESACS,0);

	if(pParams->SymbolRate <= 2000000)
		dataTime=2000;
	else if(pParams->SymbolRate <= 5000000)
		dataTime=1500;
	else if(pParams->SymbolRate <= 15000000)
		dataTime=1000;
	else
		dataTime=500;

	/* force search loop */
	ChipSetOneRegister(pParams->hDemod,RSTB0899_DSTATUS2,0x00);
	
	/* warning : vit locked has to be tested before end_loop */
	while(!(lock = ChipGetField(pParams->hDemod,FSTB0899_LOCKEDVIT)) &&	
		!ChipGetField(pParams->hDemod,FSTB0899_END_LOOPVIT) && index<dataTime)
		index++;   /* wait for viterbi end loop */
	
	/*DVB Mode*/
	if (lock)/*Test DATA LOCK indicator*/
		pParams->State = DATAOK;

	return pParams->State;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_TimingTimeConstant
--ACTION	::	Compute the amount of time needed by the timing loop to lock
--PARAMS IN	::	SymbolRate->symbol rate value
--PARAMS OUT	::	NONE
--RETURN	::	Timing loop time constant (ms)
--***************************************************/
static long FE_STB0899_TimingTimeConstant(long SymbolRate)
{
	if(SymbolRate > 0)
		return (100000/(SymbolRate/1000));
	else
		return 0;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_DerotTimeConstant
--ACTION	::	Compute the amount of time needed by the Derotator to lock
--PARAMS IN	::	SymbolRate->symbol rate value
--PARAMS OUT	::	NONE
--RETURN	::	Derotator time constant (ms)
--***************************************************/
static long FE_STB0899_DerotTimeConstant(long SymbolRate)
{
	if(SymbolRate > 0)
		return (100000/(SymbolRate/1000));
	else
		return 0;
}

/****************************************************
**FUNCTION	::	FE_STB0899_GetRollOff
**ACTION	::	Read the rolloff value
**PARAMS IN	::	hChip==>Handle for the chip
**PARAMS OUT	::	NONE
**RETURN	::	rolloff
*****************************************************/
static int FE_STB0899_GetAlpha(STCHIP_Handle_t hChip)
{
	if (ChipGetField(hChip,FSTB0899_MODE_COEF) == 1)
		return 20;
	else
		return 35;
}

/*****************************************************
**FUNCTION	::	BinaryFloatDiv
**ACTION	::	float division (with integer) 
**PARAMS IN	::	NONE
**PARAMS OUT	::	NONE
**RETURN	::	Derotator frequency (KHz)
*****************************************************/
static long BinaryFloatDiv(long n1, long n2, int precision)
{
	int i=0;
	long result=0;
	
	/*division de N1 par N2 avec N1<N2*/
	while(i<=precision) /*n1>0*/
	{
		if(n1<n2)
		{
			result*=2;
			n1*=2;
		}
		else
		{
			result=result*2+1;
			n1=(n1-n2)*2;
		}
		i++;
	}
	return result;
}

/*****************************************************
**FUNCTION	::	FE_STB0899_SetSymbolRate
**ACTION	::	Set symbol frequency
**PARAMS IN	::	hChip->handle to the chip
**			MasterClock->Masterclock frequency (Hz)
**			SymbolRate->symbol rate (bauds)
**PARAMS OUT	::	NONE
**RETURN	::	Symbol frequency
*****************************************************/
static u32 FE_STB0899_SetSymbolRate(STCHIP_Handle_t hChip, u32 MasterClock, u32 SymbolRate)
{
	u32 U32Tmp, U32TmpUp, SymbolRateUp = SymbolRate;
	
	/*
	** in order to have the maximum precision, the symbol rate entered into
	** the chip is computed as the closest value of the "true value".
	** In this purpose, the symbol rate value is rounded (1 is added on the bit
	** below the LSB )
	*/
	
	SymbolRateUp += ((SymbolRateUp * 3) / 100);
	U32Tmp = BinaryFloatDiv(SymbolRate, MasterClock, 20);
	U32TmpUp = BinaryFloatDiv(SymbolRateUp, MasterClock, 20);
	
	ChipSetFieldImage(hChip, FSTB0899_SYMB_FREQ_UP_HSB, (U32TmpUp >> 12) & 0xFF);
	ChipSetFieldImage(hChip, FSTB0899_SYMB_FREQ_UP_MSB, (U32TmpUp >> 4)& 0xFF);
	ChipSetFieldImage(hChip, FSTB0899_SYMB_FREQ_UP_LSB, U32TmpUp & 0x0F);

	ChipSetFieldImage(hChip, FSTB0899_SYMB_FREQ_HSB, (U32Tmp >> 12) & 0xFF);
	ChipSetFieldImage(hChip, FSTB0899_SYMB_FREQ_MSB, (U32Tmp >> 4) & 0xFF);
	ChipSetFieldImage(hChip, FSTB0899_SYMB_FREQ_LSB, U32Tmp & 0x0F);
	

	ChipSetRegisters(hChip,RSTB0899_SFRUPH,3);
	ChipSetRegisters(hChip,RSTB0899_SFRH,3);
	
	/*ChipSetOneRegister(hChip,RSTB0899_TMGCFG,0x40);*/
	
	return(SymbolRate) ;
}

/*****************************************************
--FUNCTION	::	CarrierWidth
--ACTION	::	Compute the width of the carrier
--PARAMS IN	::	SymbolRate->Symbol rate of the carrier (Kbauds or Mbauds)
--			RollOff	->Rolloff * 100
--PARAMS OUT	::	NONE
--RETURN	::	Width of the carrier (KHz or MHz) 
--***************************************************/
static long CarrierWidth(long SymbolRate, long RollOff)
{
	return (SymbolRate  + (SymbolRate * RollOff) / 100);
}

/*****************************************************
--FUNCTION	::	FE_STB0899_InitialCalculations
--ACTION	::	Set Params fields that are never changed during search algorithm   
--PARAMS IN	::	NONE
--PARAMS OUT	::	NONE
--RETURN	::	NONE
--***************************************************/
static void FE_STB0899_InitialCalculations(FE_STB0899_InternalParams_t *pParams)
{
	int MasterClock;

	/*Read registers (in burst mode)*/
	ChipGetOneRegister(pParams->hDemod, RSTB0899_AGC1CN);
	/*Read AGC1R and AGC2O registers */
	ChipGetRegisters(pParams->hDemod, RSTB0899_AGC1REF, 2);
	
	/*Initial calculations*/
	MasterClock = FE_STB0899_GetMclkFreq(pParams->hDemod, pParams->Quartz);
	pParams->Tagc1 = 0;
	pParams->Tagc2 = 0;
	pParams->MasterClock = MasterClock;
	pParams->Mclk = MasterClock / 65536L;
	pParams->RollOff = FE_STB0899_GetAlpha(pParams->hDemod);

	/*DVBS2 Initial calculations */
	/*Set AGC init value to to the midle*/
	pParams->AgcGain = 8154;
	ChipSetFieldImage(pParams->hDemod, FSTB0899_IF_GAININIT, pParams->AgcGain);
	ChipSetRegisters(pParams->hDemod, RSTB0899_IFAGCCNTRL, 1);
	
	pParams->RrcAlpha = (FE_DVBS2_RRCAlpha_t)ChipGetField(pParams->hDemod, FSTB0899_RRC_ALPHA);
	pParams->AcqMode = CORR_PEAK;
	pParams->FreqRange = 4; /*pSearch->FreqRange*/
	pParams->CenterFreq = 0;
	pParams->mod = DVBS2_PSK8;
	pParams->AveFrameCoarse = 10;
	pParams->AveFramefine = 20;
	pParams->AgcThreshold = 23;
	pParams->AveFrameCoarseAcq = 4;
	pParams->AveFramefineAcq = 6;
	pParams->AveFrameCoarseTrq = 10;
	pParams->AveFramefineTrq = 20;
	pParams->AutoReacq = 1;
	pParams->TracklockSel = 0;
	pParams->Zigzag = 1;
	pParams->StepSize = 2;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_SearchTiming
--ACTION	::	Perform an Fs/2 zig zag to found timing
--PARAMS IN	::	NONE
--PARAMS OUT	::	NONE
--RETURN	::	NOTIMING if no valid timing had been found, TIMINGOK otherwise
--***************************************************/
static FE_STB0899_SIGNALTYPE_t FE_STB0899_SearchTiming(FE_STB0899_InternalParams_t *pParams)
{
	short int DerotStep,
		DerotFreq = 0,
		DerotLimit,
		NextLoop = 3;
	int 	index = 0;
	
	pParams->State = NOTIMING;
	
	/* timing loop computation & symbol rate optimisation	*/
	DerotLimit = (short int)((pParams->SubRange / 2L) / pParams->Mclk);
	DerotStep = (short int)((pParams->SymbolRate / 2L) / pParams->Mclk);
	
	while((FE_STB0899_CheckTiming(pParams) != TIMINGOK) && NextLoop)
	{
		index++;
		/*Compute the next derotator position for the zig zag*/
		DerotFreq += index*pParams->Direction * DerotStep;	
		
		if(ABS(DerotFreq) > DerotLimit)
			NextLoop--;
		
		if(NextLoop)
		{
			ChipSetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_MSB, MSB(pParams->hTuner->IQ_Wiring * DerotFreq));
			ChipSetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_LSB, LSB(pParams->hTuner->IQ_Wiring * DerotFreq));
			/*Set the derotator frequency*/
			ChipSetRegisters(pParams->hDemod, RSTB0899_CFRM, 2);
		}
		/*Change the zigzag direction*/
		pParams->Direction = -pParams->Direction;	
	}
	
	if(pParams->State == TIMINGOK)
	{
		pParams->Results.SymbolRate = pParams->SymbolRate;
		/*Get the derotator frequency*/
		ChipGetRegisters(pParams->hDemod, RSTB0899_CFRM, 2);
		pParams->DerotFreq = pParams->hTuner->IQ_Wiring * ((short int) MAKEWORD(ChipGetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_MSB), ChipGetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_LSB)));
	}
	
	return pParams->State;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_SearchCarrier
--ACTION	::	Search a QPSK carrier with the derotator
--PARAMS IN	::	
--PARAMS OUT	::	NONE
--RETURN	::	NOCARRIER if no carrier had been found, CARRIEROK otherwise 
--***************************************************/
static FE_STB0899_SIGNALTYPE_t FE_STB0899_SearchCarrier(FE_STB0899_InternalParams_t *pParams)
{
	short int DerotFreq = 0,
		LastDerotFreq = 0,
		DerotLimit,
		NextLoop = 3;
	int 	index = 0;
			
	pParams->State = NOCARRIER;	
	
	DerotLimit = (short int)((pParams->SubRange / 2L) / pParams->Mclk);
	DerotFreq = pParams->DerotFreq;
	
	ChipSetField(pParams->hDemod, FSTB0899_CFD_ON, 1);
	
	do
	{
		if(FE_STB0899_CheckCarrier(pParams) == NOCARRIER)
		{
			index++;
			LastDerotFreq = DerotFreq;
			/*Compute the next derotator position for the zig zag*/
			DerotFreq += index * pParams->Direction * pParams->DerotStep;

			if(ABS(DerotFreq) > DerotLimit)
				NextLoop--;
				
			if(NextLoop)
			{
				ChipSetField(pParams->hDemod, FSTB0899_CFD_ON, 1);
				ChipSetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_MSB, MSB(pParams->hTuner->IQ_Wiring * DerotFreq));
				ChipSetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_LSB, LSB(pParams->hTuner->IQ_Wiring * DerotFreq));
				/*Set the derotator frequency*/ 
				ChipSetRegisters(pParams->hDemod, RSTB0899_CFRM, 2);
			}
		}
		else
		{
			pParams->Results.SymbolRate = pParams->SymbolRate;
		}
		/*Change the zigzag direction*/
		pParams->Direction = -pParams->Direction;	
	}
	while((pParams->State != CARRIEROK) && NextLoop);
	
	if(pParams->State == CARRIEROK)
	{
		/*Get the derotator frequency*/
		ChipGetRegisters(pParams->hDemod, RSTB0899_CFRM, 2);
		pParams->DerotFreq = pParams->hTuner->IQ_Wiring * ((short int) MAKEWORD(ChipGetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_MSB), ChipGetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_LSB)));
	}
	else
	{
		pParams->DerotFreq = LastDerotFreq;
	}
	
	return pParams->State;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_SearchData
--ACTION	::	Search a QPSK carrier with the derotator, even if there is a false 			lock 
--PARAMS IN	::	
--PARAMS OUT	::	NONE
--RETURN	::	NOCARRIER if no carrier had been found, CARRIEROK otherwise 
--***************************************************/
static FE_STB0899_SIGNALTYPE_t FE_STB0899_SearchData(FE_STB0899_InternalParams_t *pParams)
{
	short int DerotFreq,
		DerotStep,
		DerotLimit,
		NextLoop = 3;
	int 	index = 1;
	
	DerotStep = (short int)((pParams->SymbolRate / 4L) / pParams->Mclk);
	DerotLimit = (short int)((pParams->SubRange / 2L) / pParams->Mclk);
	DerotFreq = pParams->DerotFreq;
	
	do
	{
		if((pParams->State != CARRIEROK) || (FE_STB0899_CheckData(pParams) != DATAOK))
		{
			/*Compute the next derotator position for the zig zag*/
			DerotFreq += index * pParams->Direction * DerotStep;
		
			if(ABS(DerotFreq) > DerotLimit)
				NextLoop--;
			
			if(NextLoop)
			{
				ChipSetField(pParams->hDemod, FSTB0899_CFD_ON,1);
			
				ChipSetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_MSB, MSB(pParams->hTuner->IQ_Wiring * DerotFreq));
				ChipSetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_LSB, LSB(pParams->hTuner->IQ_Wiring * DerotFreq));
				/*Reset the derotator frequency*/
				ChipSetRegisters(pParams->hDemod, RSTB0899_CFRM, 2);
				FE_STB0899_CheckCarrier(pParams);
			
				index++;
			}
		}
		pParams->Direction = -pParams->Direction;/*Change the zigzag direction*/
	}
	while((pParams->State != DATAOK) && NextLoop);
	
	if(pParams->State == DATAOK)
	{
		/*Get the derotator frequency*/
		ChipGetRegisters(pParams->hDemod, RSTB0899_CFRM, 2);
		pParams->DerotFreq = pParams->hTuner->IQ_Wiring * ((short int) MAKEWORD(ChipGetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_MSB), ChipGetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_LSB)));
	}
	
	return pParams->State;
}

/****************************************************
--FUNCTION	::	FE_STB0899_CheckRange
--ACTION	::	Check if the founded frequency is in the correct range
--PARAMS IN	::	pParams->BaseFreq =>
--PARAMS OUT	::	pParams->State	=>Result of the check
--RETURN	::	RANGEOK if check success, OUTOFRANGE otherwise 
--***************************************************/
static FE_STB0899_SIGNALTYPE_t FE_STB0899_CheckRange(FE_STB0899_InternalParams_t *pParams)
{
	int	RangeOffset,
		TransponderFrequency;

	RangeOffset = pParams->SearchRange/2000;
	TransponderFrequency = pParams->Frequency + (pParams->DerotFreq * pParams->Mclk) / 1000;

	if((TransponderFrequency >= pParams->BaseFreq - RangeOffset)
	&& (TransponderFrequency <= pParams->BaseFreq + RangeOffset))
		pParams->State = RANGEOK;
	else
		pParams->State = OUTOFRANGE;
		
	return pParams->State;
}

/****************************************************
--FUNCTION	::	FirstSubRange
--ACTION	::	Compute the first SubRange of the search 
--PARAMS IN	::	pParams->SearchRange
--PARAMS OUT	::	pParams->SubRange
--RETURN	::	NONE
--***************************************************/
static void FirstSubRange(FE_STB0899_InternalParams_t *pParams)
{
	int maxsubrange;
	
	maxsubrange = TunerGetBandwidth(pParams->hTuner) - CarrierWidth(pParams->SymbolRate, pParams->RollOff) / 2;
	
	if(maxsubrange > 0)
		pParams->SubRange = MIN(pParams->SearchRange, maxsubrange);
	else
		pParams->SubRange = 0;
	pParams->Frequency = pParams->BaseFreq;
	pParams->TunerOffset = 0L;
	
	pParams->SubDir = 1;
}
/****************************************************
--FUNCTION	::	NextSubRange
--ACTION	::	Compute the next SubRange of the search 
--PARAMS IN	::	Frequency->Start frequency
--			pParams->SearchRange
--PARAMS OUT	::	pParams->SubRange
--RETURN	::	NONE
--***************************************************/
static void NextSubRange(FE_STB0899_InternalParams_t *pParams)
{
	long OldSubRange;
	
	if(pParams->SubDir > 0)
	{
		OldSubRange = pParams->SubRange;
		pParams->SubRange = MIN((pParams->SearchRange/2) - (pParams->TunerOffset + pParams->SubRange/2), pParams->SubRange);
		if(pParams->SubRange < 0)
			pParams->SubRange = 0;
		pParams->TunerOffset += (OldSubRange + pParams->SubRange) / 2;
	}
	
	pParams->Frequency = pParams->BaseFreq + (pParams->SubDir * pParams->TunerOffset) / 1000;
	pParams->SubDir = -pParams->SubDir;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_Algo
--ACTION	::	Search for Signal, Timing, Carrier and then data at a given Frequency, 
--			in a given range
--PARAMS IN	::	NONE
--PARAMS OUT	::	NONE
--RETURN	::	Type of the founded signal (if any)
--***************************************************/
FE_STB0899_SIGNALTYPE_t FE_STB0899_Algo(FE_STB0899_InternalParams_t *pParams)
{
	pParams->Frequency = pParams->BaseFreq;
	pParams->Direction = 1;

	FE_STB0899_SetSymbolRate(pParams->hDemod, pParams->MasterClock, pParams->SymbolRate);

	/* Carrier loop optimization versus symbol rate */
	if(pParams->SymbolRate <= 5000000)
		ChipSetField(pParams->hDemod, FSTB0899_ALPHA, 9);
	else
		ChipSetField(pParams->hDemod, FSTB0899_ALPHA, 7);

	if(pParams->SymbolRate <= 2000000)
		ChipSetField(pParams->hDemod, FSTB0899_BETA, 0x17);
	else if(pParams->SymbolRate <= 5000000)
		ChipSetField(pParams->hDemod, FSTB0899_BETA, 0x1C);
	else if(pParams->SymbolRate <= 15000000) 
		ChipSetField(pParams->hDemod, FSTB0899_BETA, 0x22);
	else if(pParams->SymbolRate <= 30000000)
		ChipSetField(pParams->hDemod, FSTB0899_BETA, 0x27);
	else
		ChipSetField(pParams->hDemod, FSTB0899_BETA, 0x29);
	
	/*Initial calculations*/
	/*step of DerotStep/1000 * Fsymbol*/ 
	pParams->DerotStep = pParams->DerotPercent*((S16)((pParams->SymbolRate/1000L)/pParams->Mclk));
	pParams->Ttiming = (S16)(FE_STB0899_TimingTimeConstant(pParams->SymbolRate));
	pParams->Tderot = (S16)(FE_STB0899_DerotTimeConstant(pParams->SymbolRate));
	pParams->Tdata = 500;
	
	ChipSetField(pParams->hDemod,FSTB0899_FRESRS, 1); /*Reset Stream merger*/
	
	FirstSubRange(pParams);
	
	/*Initialisations*/
	ChipSetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_MSB, 0);
	ChipSetFieldImage(pParams->hDemod, FSTB0899_CARRIER_FREQUENCY_LSB, 0);
	/*Reset of the derotator frequency*/
	ChipSetRegisters(pParams->hDemod, RSTB0899_CFRM, 2);
	ChipSetField(pParams->hDemod, FSTB0899_TIMING_LOOP_FREQ, 0xf2);
	ChipSetField(pParams->hDemod, FSTB0899_CFD_ON,1);
	
	pParams->DerotFreq = 0;
	pParams->State = NOAGC1;

	/*Move the tuner*/
	TunerSetFrequency(pParams->hTuner, pParams->Frequency);
	pParams->Frequency = TunerGetFrequency(pParams->hTuner);

	if (pParams->hTuner->Chip->ChipError)
	{
		return NOCARRIER;
	}
		
	/*Temporisations*/	
	/*Wait for agc1,agc2 and timing loop*/
	WAIT_N_MS(pParams->Tagc1 + pParams->Tagc2 + pParams->Ttiming);
	/*Is tuner Locked? (wait 100 ms maxi)*/
	WaitTuner(pParams->hTuner, 100);
		
	pParams->State = AGC1OK;	/* No AGC test actually */
		
	/*There is signal in the band*/
	if(pParams->SymbolRate <= (S32)(TunerGetBandwidth(pParams->hTuner)/2))
		FE_STB0899_SearchTiming(pParams); /*For low rates (SCPC)*/
	else
		FE_STB0899_CheckTiming(pParams); /*For high rates (MCPC)*/

	if(pParams->State == TIMINGOK)
	{	//printk(KERN_ERR "TIMINGOK\n");
		if(FE_STB0899_SearchCarrier(pParams) == CARRIEROK)	
		{	//printk(KERN_ERR "CARRIEROK\n");
			/*Check for data*/
			if(FE_STB0899_SearchData(pParams) == DATAOK)
			{	//printk(KERN_ERR "DATAOK\n");
				if(FE_STB0899_CheckRange(pParams) == RANGEOK)
				{	//printk(KERN_ERR "RANGEOK\n");
					pParams->Results.Frequency = pParams->Frequency + (pParams->DerotFreq * (pParams->Mclk) / 1000);
					pParams->Results.PunctureRate = (FE_STB0899_Rate_t)ChipGetField(pParams->hDemod, FSTB0899_VIT_CURPUN);
				}
			}
		}
	}
	if(pParams->State != RANGEOK)
		NextSubRange(pParams);
	
	ChipSetField(pParams->hDemod, FSTB0899_FRESRS, 0); /*release Stream merger reset*/
	ChipSetField(pParams->hDemod, FSTB0899_CFD_ON, 0); /*Disable Carrier detector*/
	
	ChipGetRegisters(pParams->hDemod, RSTB0899_EQUAI1, 10);

	pParams->Results.SignalType = pParams->State;
	
	/*if locked and range is ok set Kdiv value*/
	if(pParams->State == RANGEOK)
	{
		switch(pParams->Results.PunctureRate)
		{
			case 13:		/*1/2*/
				ChipSetField(pParams->hDemod, FSTB0899_KDIVIDER, 0x1a/*36*/);
			break;
			
			case 18:		/*2/3*/
				ChipSetField(pParams->hDemod, FSTB0899_KDIVIDER, 44/*0x27*/);
			break;
			
			case 21:		/*3/4*/
				ChipSetField(pParams->hDemod, FSTB0899_KDIVIDER,/*0x34*/60);
			break;
			
			case 24:		/*5/6*/
				ChipSetField(pParams->hDemod, FSTB0899_KDIVIDER,75/*0x4f*/);
			break;
			
			case 25:		/*6/7*/
				ChipSetField(pParams->hDemod, FSTB0899_KDIVIDER,/*0x5c*/80);
			break;
			
			case 26:		/*7/8*/
				ChipSetField(pParams->hDemod, FSTB0899_KDIVIDER,94/*0x6a*/);
			break;	
		}
	}
	return	pParams->State;
}

FE_DVBS2_State  FE_STB0899_DVBS2Algo(FE_STB0899_InternalParams_t *pParams)
{
	FE_STB0899_DVBS2_InitParams_t initParams;
	FE_DVBS2_ReacquireParams_t racqParams;
	FE_DVBS2_ModCod_t modCode;
	
	S32 offsetfreq,searchTime,pilots;
	
	/*Init Params Initialization*/
	initParams.RRCAlpha = pParams->RrcAlpha;
	initParams.ModeMode = pParams->mod;
	initParams.SymbolRate = pParams->DVBS2SymbolRate;
	initParams.MasterClock = pParams->MasterClock;
	initParams.CarrierFrequency = 0;
	initParams.AveFrameCoarse = pParams->AveFrameCoarse;
	initParams.AveFramefine = pParams->AveFramefine;
	initParams.AgcThreshold = pParams->AgcThreshold;
	
	/*Reacquire Params initialization*/
	racqParams.AcqMode = pParams->AcqMode;
	racqParams.SymbolRate = pParams->DVBS2SymbolRate;
	racqParams.MasterClock = pParams->MasterClock;
	racqParams.FreqRange = pParams->SearchRange / 1000000/*pParams->FreqRange*/;
	racqParams.CenterFreq = pParams->CenterFreq;
	racqParams.AveFrameCoarseAcq = pParams->AveFrameCoarseAcq;
	racqParams.AveFramefineAcq = pParams->AveFramefineAcq;
	racqParams.AveFrameCoarseTrq = pParams->AveFrameCoarseTrq;
	racqParams.AveFramefineTrq = pParams->AveFramefineTrq;
	racqParams.AutoReacq = pParams->AutoReacq;
	racqParams.TracklockSel = pParams->TracklockSel;
	racqParams.Zigzag = pParams->Zigzag;
	racqParams.StepSize = pParams->StepSize;
	
	if(pParams->DVBS2SymbolRate <= 2000000)
		searchTime = 5000;
	else if(pParams->DVBS2SymbolRate <= 5000000)
		searchTime = 2500;
	else if(pParams->DVBS2SymbolRate <= 10000000)
		searchTime = 1500;
	else if  (pParams->DVBS2SymbolRate <= 15000000)
		searchTime = 700;
	else if  (pParams->DVBS2SymbolRate <= 20000000)
		searchTime = 500;
	else if  (pParams->DVBS2SymbolRate <= 25000000)
		searchTime = 200;
	else
		searchTime = 150;

	/*move tuner*/
	TunerSetFrequency(pParams->hTuner, pParams->Frequency);	
	pParams->Frequency = TunerGetFrequency(pParams->hTuner);
	/*Temporisations*/
	WaitTuner(pParams->hTuner, 100);	 /*Is tuner Locked?(wait 100 ms maxi)*/ 

	/*Set IF AGC to Acquire value*/
	ChipSetFieldImage(pParams->hDemod, FSTB0899_IF_LOOPGAIN, 4);
	ChipSetRegisters(pParams->hDemod, RSTB0899_IFAGCCNTRL, 1);
	
	ChipSetFieldImage(pParams->hDemod, FSTB0899_IF_AGC_DUMPPER, 0);
	ChipSetRegisters(pParams->hDemod, RSTB0899_IFAGCCNTRL2, 1);

	/*Initialisations*/
	FE_DVBS2_InitialCalculations(pParams->hDemod, &initParams);
	
	/*IQ swap setting*/
	if((pParams->SpectralInv == FE_IQ_AUTO) || (pParams->SpectralInv == FE_IQ_NORMAL))
	{
		/* I,Q Spectrum Set to Normal*/ 
		ChipSetFieldImage(pParams->hDemod, FSTB0899_SPECTRUM_INVERT, 0);
		ChipSetRegisters(pParams->hDemod, RSTB0899_DMDCNTRL2, 1);
	}
	else
	{
		/* I,Q Spectrum Inverted*/ 
		ChipSetFieldImage(pParams->hDemod, FSTB0899_SPECTRUM_INVERT, 1);
		ChipSetRegisters(pParams->hDemod, RSTB0899_DMDCNTRL2, 1);
	}
		
	FE_DVBS2_Reacquire(pParams->hDemod, &racqParams);
	/*Wait for UWP,CSM and DATA LOCK searchTime ms max*/
	pParams->Results.DVBS2SignalType = pParams->DVBS2State = FE_DVBS2_GetState(pParams->hDemod, searchTime);
	
	if(pParams->DVBS2State != FE_DVBS2_DATAOK)
	{
		if(pParams->SpectralInv == FE_IQ_AUTO)
		{
			/* I,Q Spectrum Invertion*/ 
			ChipSetFieldImage(pParams->hDemod, FSTB0899_SPECTRUM_INVERT, 1);
			ChipSetRegisters(pParams->hDemod, RSTB0899_DMDCNTRL2, 1);
			/* start acquistion process  */
			FE_DVBS2_Reacquire(pParams->hDemod, &racqParams);
			/*Whait for UWP,CSM and data LOCK 200ms max*/
			pParams->Results.DVBS2SignalType = pParams->DVBS2State = FE_DVBS2_GetState(pParams->hDemod, searchTime);
			if(pParams->DVBS2State == FE_DVBS2_DATAOK)
				pParams->SpectralInv = FE_IQ_SWAPPED;
		}
	}
	/*Set IF AGC to tracking value*/
	ChipSetFieldImage(pParams->hDemod, FSTB0899_IF_LOOPGAIN, 3);
	ChipSetRegisters(pParams->hDemod, RSTB0899_IFAGCCNTRL, 1);
	
	ChipSetFieldImage(pParams->hDemod, FSTB0899_IF_AGC_DUMPPER, 7);
	ChipSetRegisters(pParams->hDemod, RSTB0899_IFAGCCNTRL2, 1);

	if(pParams->DVBS2State == FE_DVBS2_DATAOK)
	{
		modCode = (FE_DVBS2_ModCod_t)FE_DVBS2_GetModCod(pParams->hDemod);
		pilots=ChipGetFieldImage(pParams->hDemod, FSTB0899_UWP_DECODED_MODCODE) & 0x01;

		if((((10*pParams->MasterClock) / (pParams->DVBS2SymbolRate/10)) <= 400) && (INRANGE(FE_QPSK_12, modCode, FE_QPSK_910)) && (pilots == 1))
		{
			FE_DVBS2_CSMInitialize(pParams->hDemod, pilots, modCode, pParams->DVBS2SymbolRate, pParams->MasterClock);
			/*Wait for UWP,CSM and data LOCK 20ms max*/
			pParams->Results.DVBS2SignalType = pParams->DVBS2State = FE_DVBS2_GetState(pParams->hDemod, 20);
		}

		/*if locked store signal parameters*/
		offsetfreq = ChipGetField(pParams->hDemod, FSTB0899_CRL_FREQUENCY);
		offsetfreq = offsetfreq / (PowOf2(30) / 1000);
		offsetfreq *= (pParams->MasterClock / 1000000);
		if(ChipGetFieldImage(pParams->hDemod, FSTB0899_SPECTRUM_INVERT))
			offsetfreq *= -1;
		
		pParams->Results.Frequency = TunerGetFrequency(pParams->hTuner) - offsetfreq;
  		pParams->Results.DVBS2SymbolRate = FE_DVBS2_GetSymbolRate(pParams->hDemod, pParams->MasterClock);
		pParams->Results.ModCode = (FE_DVBS2_ModCod_t)FE_DVBS2_GetModCod(pParams->hDemod);
		pParams->Results.Pilots = (BOOL)ChipGetFieldImage(pParams->hDemod, FSTB0899_UWP_DECODED_MODCODE) & 0x01;
		pParams->Results.FrameLength = (FE_DVBS2_FRAME)((ChipGetFieldImage(pParams->hDemod, FSTB0899_UWP_DECODED_MODCODE) >> 1) & 0x01);
		
		/*Set AGC init value to to the founded AGC level*/
		pParams->AgcGain = ChipGetField(pParams->hDemod, FSTB0899_IF_AGCGAIN);
		ChipSetFieldImage(pParams->hDemod, FSTB0899_IF_GAININIT, pParams->AgcGain);
		ChipSetRegisters(pParams->hDemod, RSTB0899_IFAGCCNTRL, 1);

		/*if QPSK 1/2 ,QPSK 3/5 or QPSK 2/3 set IF AGC reference to 16 otherwise 32*/
		if(pParams->Results.ModCode <= FE_QPSK_23)
		{
			ChipSetFieldImage(pParams->hDemod, FSTB0899_IF_AGCREF, 16);
			ChipSetRegisters(pParams->hDemod, RSTB0899_IFAGCCNTRL, 1);
		}
		else
		{
			ChipSetFieldImage(pParams->hDemod,FSTB0899_IF_AGCREF,32);
			ChipSetRegisters(pParams->hDemod,RSTB0899_IFAGCCNTRL,1);
		}
	}
	return	pParams->DVBS2State;
}

/*Symbol Rate in Hz,Mclk in Hz */
static void FE_STB0899_SetIterScal(STCHIP_Handle_t hChip, u32 MasterClock, u32 SymbolRate)
{
	S32 iTerScal;
	
	iTerScal = 17 * (MasterClock / 1000);
	iTerScal += 410000;
	iTerScal /= (SymbolRate / 1000000);
	iTerScal /= 1000;
	
	if(iTerScal > 150)
		iTerScal = 150;
	
	ChipSetField(hChip, FSTB0899_ITERATION_SCALE, iTerScal);
}

/*****************************************************
--FUNCTION	::	FE_STB0899_Init
--ACTION	::	Initialisation of the STB0899 chip
--PARAMS IN	::	pInit==>pointer to stb0899_state structure
--PARAMS OUT	::	NONE
--RETURN	::	Handle to STB0899
--***************************************************/
FE_STB0899_Handle_t  FE_STB0899_Init(FE_STB0899_InitParams_t *pInit)
{
	FE_STB0899_InternalParams_t *pParams = NULL;

	/* Internal params structure allocation */
	pParams = (FE_STB0899_InternalParams_t *)kmalloc(sizeof(FE_STB0899_InternalParams_t), GFP_KERNEL);

	if(pParams != NULL)
	{
		/* Chip initialisation */
		pParams->hDemod = STB0899_Init(pInit->STB0899Init);
		
		if(pInit->TunerInit->Chip->Repeater)
			pInit->TunerInit->Chip->RepeaterHost = pParams->hDemod;
		pParams->hTuner = STTunerInit(pInit->TunerInit);
		pParams->hLnb = LNBP21_Init(pInit->LnbInit);
		
		if((pParams->hDemod != NULL) && (!pParams->hDemod->ChipError))
		{
			FE_STB0899_SetStandard(pParams->hDemod, (STB0899_STANDARD)pInit->Standard);

			pParams->Quartz = 27000000;
				
			switch(pInit->Clock)
			{
				case FE_PARALLEL_CLOCK:	
				/*TS_CLK = MCLK , if parallel TS disable output fifo */	
					ChipSetField(pParams->hDemod, FSTB0899_OUTRS_PS, 0x00);
					ChipSetOneRegister(pParams->hDemod, RSTB0899_TSOUT, 0x7f);
				break;
				
				case FE_SERIAL_MASTER_CLOCK:
				/*TS_CLK = MCLK if serial  TS do not disable output fifo , set TS_clk ratio to 1 */
					ChipSetField(pParams->hDemod, FSTB0899_OUTRS_PS, 0x01);
					ChipSetOneRegister(pParams->hDemod, RSTB0899_TSOUT, 0x23);
				break;
				
				default:
				break;
			}

			switch(pInit->Parity)
			{
				case FE_PARITY_ON:
					ChipSetField(pParams->hDemod, FSTB0899_CLK_POL, 0x00);
				break;
				
				case FE_PARITY_OFF: 
					ChipSetField(pParams->hDemod, FSTB0899_CLK_POL, 0x01);
				break;
				
				default:
				break;
			}
			FE_STB0899_InitialCalculations(pParams);
		}
		else
		{
			kfree(pParams);
			pParams = NULL;
		}
	}
	return  (FE_STB0899_Handle_t) pParams;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_SetMclk
--ACTION	::	Set demod Master Clock  
--PARAMS IN	::	Handle==>Front End Handle
		::	Mclk : demod master clock
		::	ExtClk external Quartz
--PARAMS OUT	::	NONE.
--RETURN	::	Error (if any)
--***************************************************/
FE_STB0899_Error_t FE_STB0899_SetMclk(FE_STB0899_Handle_t Handle, u32 Mclk, u32 ExtClk)
{
	FE_STB0899_Error_t st_error = FE_NO_ERROR;
	FE_STB0899_InternalParams_t * pParams = (FE_STB0899_InternalParams_t *)Handle;

	u32 mDiv;
	if(pParams == NULL)
		st_error = FE_INVALID_HANDLE;
	else
	{
		mDiv = ((6 * Mclk) / ExtClk) - 1;
		ChipSetField(pParams->hDemod, FSTB0899_MDIV, mDiv); 
		pParams->MasterClock = FE_STB0899_GetMclkFreq(pParams->hDemod, pParams->Quartz);
	}
	return(FE_NO_ERROR);
}

FE_STB0899_Error_t FE_STB0899_Search(FE_STB0899_Handle_t Handle, FE_STB0899_SearchParams_t *pSearch, FE_STB0899_SearchResult_t *pResult)
{
	FE_STB0899_Error_t st_error = FE_NO_ERROR;
	FE_STB0899_InternalParams_t *pParams;

	if(Handle != 0)
	{
		pParams = (FE_STB0899_InternalParams_t *) Handle;
		FE_STB0899_SetInternalError(FE_IERR_NO, FE_LOC_NOWHERE, &pParams->Inl_Error);
		pParams->Standard = pSearch->Standard;
		if((INRANGE(1000000, pSearch->SymbolRate, 45000000)) && (INRANGE(1000000, pSearch->SearchRange, 50000000)))
		{
			printk(KERN_ERR "inrange 1-45M\n");
			FE_STB0899_SetStandard(pParams->hDemod, (STB0899_STANDARD)pSearch->Standard);
			
			/*Set tuner Gain */
			if(pSearch->SymbolRate > 15000000)
				TunerSetGain(pParams->hTuner, 8);
			else if(pSearch->SymbolRate > 5000000)
				TunerSetGain(pParams->hTuner,12);
			else
				TunerSetGain(pParams->hTuner,14);

			/*For Low Symbol Rate (<=5Mbs) set Mclk to 45MHz, else use 108MHz*/
			if(pSearch->SymbolRate <= 5000000)
			{
				FE_STB0899_SetMclk(Handle, 45000000, pParams->Quartz);
			}
			else
			{	//printk(KERN_ERR "pSearch->SymbolRate > 5M\n");
				FE_STB0899_SetMclk(Handle, 108000000, pParams->Quartz);
			}
		
			switch(pSearch->Standard)
			{
				case FE_DVBS1_STANDARD:
				case FE_DSS_STANDARD:
					printk(KERN_ERR "FE_DVBS1_STANDARD\n");
					/* Fill pParams structure with search parameters */
					pParams->BaseFreq = pSearch->Frequency;
					pParams->SymbolRate = pSearch->SymbolRate;
					pParams->SearchRange = pSearch->SearchRange;
					pParams->DerotPercent = 10;
					TunerSetBandwidth(pParams->hTuner, (13 * (CarrierWidth(pParams->SymbolRate, pParams->RollOff) + 10000000)) / 10);
					pParams->TunerBW = TunerGetBandwidth(pParams->hTuner);
					/*Set DVB-S1 AGC*/
					ChipSetOneRegister(pParams->hDemod, RSTB0899_AGCRFCFG, 0x11);
					/* Run the search algorithm */
					if((FE_STB0899_Algo(pParams) == RANGEOK) && (pParams->hDemod->ChipError == CHIPERR_NO_ERROR))
					{
						pResult->Locked = TRUE;
						/* update results */
						pResult->Frequency =pParams->Results.Frequency;
						pResult->SymbolRate = pParams->Results.SymbolRate;
						pResult->Rate = pParams->Results.PunctureRate;
					}
					else
					{
						pResult->Locked = FALSE;
					
						switch(pParams->Inl_Error.Type)
						{
							case FE_IERR_I2C:
								st_error = FE_I2C_ERROR;
							break;
				
							case FE_IERR_NO:
							default:
								st_error = FE_SEARCH_FAILED;
							break;
						}
					}
				break;
				
				case FE_DVBS2_STANDARD:
					printk(KERN_ERR "FE_DVBS2_STANDARD\n");
					/* Fill pParams structure with search parameters */
					pParams->Frequency = pSearch->Frequency;
					pParams->BaseFreq = pSearch->Frequency;
					pParams->DVBS2SymbolRate = pSearch->SymbolRate;
					pParams->SpectralInv = pSearch->IQ_Inversion;
			
					pParams->SearchRange = pSearch->SearchRange;
					TunerSetBandwidth(pParams->hTuner, (13 * (FE_DVBS2_CarrierWidth(pParams->DVBS2SymbolRate,pParams->RrcAlpha) + 10000000)) / 10);
					pParams->TunerBW = TunerGetBandwidth(pParams->hTuner);

					/*Set DVB-S2 AGC*/
					ChipSetOneRegister(pParams->hDemod, RSTB0899_AGCRFCFG, 0x1c);
					
					/*Set IterScale =f(MCLK,SYMB,MODULATION*/
					FE_STB0899_SetIterScal(pParams->hDemod, pParams->MasterClock, pParams->DVBS2SymbolRate);
					
					/* Run the DVBS2  search algorithm */
					if((FE_STB0899_DVBS2Algo(pParams) == FE_DVBS2_DATAOK) && (pParams->hDemod->ChipError == CHIPERR_NO_ERROR))
					{
						pResult->Locked = TRUE;
						/* update results */
						pResult->Frequency =pParams->Results.Frequency;
						pResult->SymbolRate = pParams->Results.DVBS2SymbolRate;
						pResult->ModCode = pParams->Results.ModCode;
						pResult->Pilots = pParams->Results.Pilots;
						pResult->FrameLength = pParams->Results.FrameLength;
					}
					else
					{
						pResult->Locked = FALSE;
				
						switch(pParams->Inl_Error.Type)
						{
							case FE_IERR_I2C:
								st_error = FE_I2C_ERROR;
							break;
				
							case FE_IERR_NO:
							default:
								st_error = FE_SEARCH_FAILED;
							break;
						}
					}
				break;
				
				default:
					st_error = FE_BAD_PARAMETER;
				break;
			}
		}
		else
			st_error = FE_BAD_PARAMETER;
	}
	else
		st_error=FE_INVALID_HANDLE;
	
	return st_error;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_GetRFLevel
--ACTION	::	Return power of the signal
--PARAMS IN	::	NONE	
--PARAMS OUT	::	NONE
--RETURN	::	Power of the signal (dBm), 0 if no signal 
--***************************************************/
u16 FE_STB0899_GetRFLevel(STCHIP_Handle_t hChip,FE_STB0899_LOOKUP_t *lookup,STB0899_STANDARD Standard)
{
	u8 Imin, Imax, i;
	S16 agcGain = 0,rfLevel = 0;
	
	if((lookup != NULL) && lookup->size)
	{
		switch(Standard)
		{
			case FE_DVBS1_STANDARD:
			case FE_DSS_STANDARD:
				agcGain = ChipGetField(hChip,FSTB0899_AGCIQ_VALUE);
			break;
			
			case FE_DVBS2_STANDARD:
				agcGain = ChipGetField(hChip,FSTB0899_IF_AGCGAIN);
			break;
		}
		Imin = 0;
		Imax = lookup->size-1;
		
		if(INRANGE(lookup->table[Imin].regval,agcGain,lookup->table[Imax].regval))
		{
			while((Imax-Imin)>1)
			{
				i=(Imax+Imin)/2;
				
				if(INRANGE(lookup->table[Imin].regval,agcGain,lookup->table[i].regval))
					Imax = i;
				else
					Imin = i;
			}
			rfLevel =(((S32)agcGain - lookup->table[Imin].regval) * (lookup->table[Imax].realval - lookup->table[Imin].realval) / (lookup->table[Imax].regval - lookup->table[Imin].regval)) + lookup->table[Imin].realval + 100;
		}
		else
			rfLevel = -100+100;
	}
	return (u16)rfLevel;
}

/*****************************************************
--FUNCTION	::	CarrierGetQuality
--ACTION	::	Return the carrier to noise of the current carrier
--PARAMS IN	::	NONE	
--PARAMS OUT	::	NONE
--RETURN	::	C/N of the carrier, 0 if no carrier 
--***************************************************/
u16 CarrierGetQuality(STCHIP_Handle_t hChip, FE_STB0899_LOOKUP_t *lookup, STB0899_STANDARD Standard)
{
	u16 regval, Imin, Imax, i;
	u16 c_n = 0, quant, val2;

	switch(Standard)
	{
		case FE_DVBS1_STANDARD:
		case FE_DSS_STANDARD:
			if(ChipGetField(hChip,FSTB0899_CARRIER_FOUND))
			{
				if((lookup != NULL) && lookup->size)
				{
					ChipGetRegisters(hChip, RSTB0899_NIRM, 2);
					ChipGetRegisters(hChip, RSTB0899_NIRL, 2);

					regval = MAKEWORD(ChipGetFieldImage(hChip, FSTB0899_NOISE_IND_MSB), ChipGetFieldImage(hChip, FSTB0899_NOISE_IND_LSB));
	
					Imin = 0;
					Imax = lookup->size-1;

					if(INRANGE(lookup->table[Imin].regval, regval, lookup->table[Imax].regval))
					{
						while((Imax-Imin) > 1)
						{
							i = (Imax + Imin) / 2;
							if(INRANGE(lookup->table[Imin].regval, regval, lookup->table[i].regval))
								Imax = i;
							else
								Imin = i;
						}	
						c_n = ((regval - lookup->table[Imin].regval) * (lookup->table[Imax].realval - lookup->table[Imin].realval) / (lookup->table[Imax].regval - lookup->table[Imin].regval)) + lookup->table[Imin].realval;

						c_n = (int)(c_n / 2);
						if (c_n >= 100)
							c_n = 98;
					}
					else if(regval < lookup->table[Imin].regval)
						c_n = 98;
				}
			}
			
		break;
		
		case FE_DVBS2_STANDARD:
			
			quant = ChipGetField(hChip, FSTB0899_UWP_ESN0_QUANT);
			c_n = FE_DVBS2_GetUWPEsNo(hChip, quant);
			if(c_n == 1)
				c_n = 301;   /*C/N = 30.1*/
			else if(c_n == 2)
				c_n = 270;   /*C/N = 27*/
			else
			{
				val2 = (long)(-10 * (Log10Int((long)(c_n)) - 2 * Log10Int((long)(quant))));
				val2 = MULT32X32(val2, 646456993L);
				val2 *= 10;
				c_n = (u16)(((unsigned long)(val2)) / PowOf2(24));
			}
		break;
	}
	
	return c_n;
}

u32 FE_STB0899_GetError(STCHIP_Handle_t hChip, u32 Standrad)
{
	u32 ber = 0,i;
	
	switch(Standrad)
	{
		case FE_DVBS1_STANDARD:
		case FE_DSS_STANDARD:
			/* force to viterbi bit error */
			ChipSetOneRegister(hChip, RSTB0899_ERRCTRL1, 0x3D);
			ChipGetOneRegister(hChip, RSTB0899_VSTATUS);
	
			/* Average 5 ber values */
			WAIT_N_MS(1000);
			for(i=0; i<5; i++)
			{
				WAIT_N_MS(100);
				ber += FE_STB0899_GetErrorCount(hChip, COUNTER1);
			}
	
			ber /= 5;
			/*Check for carrier*/
			if(ChipGetFieldImage(hChip, FSTB0899_PRFVIT))
			{
				/*Error Rate*/
				ber *= 9766;
				/*theses two lines => ber = ber * 10^7*/
				ber /= (-1 + PowOf2(0 + 2*ChipGetFieldImage(hChip, FSTB0899_NOE)));
				ber /= 8;
			}
		break;
		
		case FE_DVBS2_STANDARD:
			/*force to DVBS2 PER*/
			ChipSetOneRegister(hChip, RSTB0899_ERRCTRL1, 0xB6);
			ChipGetOneRegister(hChip, RSTB0899_VSTATUS);
	
			/*Average 5 ber values*/
			for(i=0; i<5; i++)
			{
				WAIT_N_MS(100);
				ber += FE_STB0899_GetErrorCount(hChip, COUNTER1);
			}
			
			ber *= 10000000;
			ber /= (-1 + PowOf2(4 + 2 * ChipGetFieldImage(hChip, FSTB0899_NOE)));
		break;
	}
	return ber;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_GetSignalInfo
--ACTION	::	Return informations on the locked transponder
--PARAMS IN	::	Handle	==>Front End Handle
--PARAMS OUT	::	pInfo	==> Informations (BER,C/N,power ...)
--RETURN	::	Error (if any)
--***************************************************/
FE_STB0899_Error_t FE_STB0899_GetSignalInfo(FE_STB0899_Handle_t Handle, FE_STB0899_SignalInfo_t	*pInfo)
{
    FE_STB0899_Error_t st_error = FE_NO_ERROR;
    FE_STB0899_InternalParams_t *pParams = NULL;

    pParams = (FE_STB0899_InternalParams_t *) Handle;
	
    if(pParams != NULL)
    {
    	switch(pParams->Standard)
	{
	    case FE_DVBS1_STANDARD:
	    case FE_DSS_STANDARD:

		pInfo->Locked = ChipGetField(pParams->hDemod, FSTB0899_LOCKEDVIT);
		if(pInfo->Locked)
		{
		    pInfo->Power = FE_STB0899_GetRFLevel(pParams->hDemod, &FE_STB0899_RF_LookUp, (STB0899_STANDARD)pParams->Standard);
		    pInfo->C_N = CarrierGetQuality(pParams->hDemod, &FE_STB0899_CN_LookUp, (STB0899_STANDARD)pParams->Standard);
		    pInfo->BER = FE_STB0899_GetError(pParams->hDemod, pParams->Standard);
		}
	    break;
	
	    case FE_DVBS2_STANDARD:
		pInfo->Locked = ((FE_DVBS2_GetState(pParams->hDemod, 10) == FE_DVBS2_DATAOK) ? 1 : 0);
		{
		    pInfo->C_N = CarrierGetQuality(pParams->hDemod, &FE_STB0899_CN_LookUp, (STB0899_STANDARD)pParams->Standard);
		    pInfo->Power = FE_STB0899_GetRFLevel(pParams->hDemod, &FE_STB0899_DVBS2RF_LookUp, (STB0899_STANDARD)pParams->Standard);
		    pInfo->BER = FE_STB0899_GetError(pParams->hDemod, pParams->Standard);
		}
	    break;
	}
    }
    else
	st_error = FE_INVALID_HANDLE;
	
    return st_error;
}

FE_STB0899_Error_t FE_STB0899_DiseqcSend(FE_STB0899_Handle_t Handle, u8 *Data, u32 NbData)
{
	FE_STB0899_Error_t st_error = FE_NO_ERROR;
	FE_STB0899_InternalParams_t *pParams = NULL;
	
	pParams = (FE_STB0899_InternalParams_t *)Handle;
	
	if(pParams != NULL)
	{
		u32 i=0;

		ChipSetField(pParams->hDemod, FSTB0899_DISPRECHARGE, 1);
		while(i<NbData)
		{
		    	/*wait for FIFO empty*/
			while(ChipGetField(pParams->hDemod, FSTB0899_FIFOPARITYFAIL));
			/*send byte to FIFO::WARNING don't use set field!!*/
			ChipSetOneRegister(pParams->hDemod, RSTB0899_DISFIFO, Data[i]);
			i++;
		}
		ChipSetField(pParams->hDemod, FSTB0899_DISPRECHARGE, 0);
	}
	else
		st_error = FE_INVALID_HANDLE;

	return st_error;
}

/*****************************************************
--FUNCTION	::	FE_STB0899_Set22KHZContinues
--ACTION	::	Initialize DiseqC 
--PARAMS IN	::	Handle	==> Front End Handle
			ToneOn	==> 22 KHz on Off
--PARAMS OUT	::	RxFreq	==> None.
--RETURN	::	Error (if any)
--***************************************************/
FE_STB0899_Error_t FE_STB0899_Set22KHZContinues(FE_STB0899_Handle_t Handle, BOOL ToneOn)
{
	u32 mclk, div;
	FE_STB0899_Error_t st_error = FE_NO_ERROR;
	FE_STB0899_InternalParams_t *pParams = NULL;

	pParams = (FE_STB0899_InternalParams_t *)Handle;
	if(pParams != NULL)
	{
	    switch(ToneOn)
	    {
		case SEC_TONE_ON:
		    mclk=FE_STB0899_GetMclkFreq(pParams->hDemod, pParams->Quartz);
		    div = (mclk / 100) / (2 * 32 * 22 * 4);
		    div = (div + 5) / 10;
		    /*Route DiseqC Tx pin to AuxClock0 */
		    ChipSetOneRegister(pParams->hDemod, RSTB0899_DISEQCOCFG, 0x66);
		    ChipSetField(pParams->hDemod, FSTB0899_ACRPRESC, 3);
		    ChipSetField(pParams->hDemod, FSTB0899_ACRDIV1, div);
		break;

		case SEC_TONE_OFF:
		    /*Set Diseqc pin to general diseq mod*/
		    ChipSetOneRegister(pParams->hDemod, RSTB0899_DISEQCOCFG, 0x20);
		break;
		}
	}
	else
		st_error = FE_INVALID_HANDLE;

	return st_error;
}

STCHIP_Error_t STB0899_RepeaterFn(STCHIP_Handle_t hChip, BOOL State)
{
	STCHIP_Error_t ch_error = CHIPERR_NO_ERROR;
	
	if(hChip != NULL)
	{
		if(State == TRUE)
			ChipSetField(hChip, FSTB0899_I2CTON, 1);
	}
	
	return ch_error;
}






