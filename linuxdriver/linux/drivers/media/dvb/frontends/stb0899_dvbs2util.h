#ifndef STB0899_DVBS2UTIL_H
#define STB0899_DVBS2UTIL_H

#include "stb0899_common.h"
#include "stb0899_chip.h"

#define BTR_NCO_BITS    28
#define CRL_NCO_BITS    30

#define CRL_GAIN_SHIFT_OFFSET 11
#define BTR_GAIN_SHIFT_OFFSET 15

#define ESNO_AVE            3
#define ESNO_QUANT          32
#define AVEFRAMES_COARSE    10
#define AVEFRAMES_FINE      20
#define MISS_THRESHOLD      6
#define UWP_THRESHOLD_ACQ   1125
#define UWP_THRESHOLD_TRACK 758
#define UWP_THRESHOLD_SOF   1350
#define SOF_SEARCH_TIMEOUT  1664100

typedef enum
{
	CORR_PEAK,
	MIN_FREQ_EST,
	UWP_LOCK,
	FEC_LOCK,
	NO_SEARCH
}FE_DVBS2_AcqMode;

typedef enum
{ 
	DVBS2_BPSK,
	DVBS2_QPSK,
	DVBS2_OQPSK,
	DVBS2_PSK8
}FE_DVBS2_Mode_t;
	
typedef enum
{
	FE_DUMMY_PLF,
	FE_QPSK_14,
	FE_QPSK_13,
	FE_QPSK_25,
	FE_QPSK_12,
	FE_QPSK_35,
	FE_QPSK_23,
	FE_QPSK_34,
	FE_QPSK_45,
	FE_QPSK_56,
	FE_QPSK_89,
	FE_QPSK_910,
	FE_8PSK_35,
	FE_8PSK_23,
	FE_8PSK_34,
	FE_8PSK_56,
	FE_8PSK_89,
	FE_8PSK_910,
	FE_16APSK_23,
	FE_16APSK_34,
	FE_16APSK_45,
	FE_16APSK_56,
	FE_16APSK_89,
	FE_16APSK_910,
	FE_32APSK_34,
	FE_32APSK_45,
	FE_32APSK_56,
	FE_32APSK_89,
	FE_32APSK_910
}FE_DVBS2_ModCod_t;
	
typedef enum
{
	FE_LONG_FRAME,
	FE_SHORT_FRAME
}FE_DVBS2_FRAME;

typedef enum
{
	RRC_20,
	RRC_25,
	RRC_35
}FE_DVBS2_RRCAlpha_t;
	
typedef enum
{
	FE_DVBS2_NOAGC,
	FE_DVBS2_AGCOK,
	FE_DVBS2_TIMINGOK,
	FE_DVBS2_NOTIMING,
	FE_DVBS2_NOCARRIER,
	FE_DVBS2_CARRIEROK,
	FE_DVBS2_NOUWP,
	FE_DVBS2_UWPOK,
	FE_DVBS2_NODATA,
	FE_DVBS2_DATAOK	
}FE_DVBS2_State;

typedef struct
{
	u32	LoopBwPercent;
	u32	SymbolRate;
	u32	MasterClock;
	FE_DVBS2_Mode_t Mode;
	u32	Zeta;
	u32 	SymPeakVal;
}FE_DVBS2_LoopBW_Params_t;

typedef struct
{
	u32	Adapt;
	u32	AmplImbEstim;
	u32	PhsImbEstim;
	u32	AmplAdaptLsht;
	u32	PhsAdaptLsht;
}FE_DVBS2_ImbCompInit_Params_t;

typedef struct
{
	S32 	EsNoAve,
		EsNoQuant,
		AveFramesCoarse,
		AveframesFine,
		MissThreshold,
		ThresholdAcq,
		ThresholdTrack,
		ThresholdSof,
		SofSearchTimeout;
}FE_DVBS2_UWPConfig_Params_t;

typedef struct
{
	S32	DvtTable,
		TwoPass,
		AgcGain,
		AgcShift,
		FeLoopShift,
		GammaAcq,
		GammaRhoAcq,
		GammaTrack,
		GammaRhoTrack,
		LockCountThreshold,
		PhaseDiffThreshold;
}FE_DVBS2_CSMConfig_Params_t;

typedef struct
{
	FE_DVBS2_RRCAlpha_t RRCAlpha;
	FE_DVBS2_Mode_t	ModeMode;
	
	S32	SymbolRate,
		MasterClock,
		CarrierFrequency,
		AveFrameCoarse,
		AveFramefine,
		AgcThreshold;
	
	u16	SpectralInv;
} FE_STB0899_DVBS2_InitParams_t;

typedef struct
{
	FE_DVBS2_AcqMode AcqMode;
	/*FE_DVBS2_Mode_t mod;*/

	u32 	SymbolRate,
		MasterClock,
		FreqRange,
		CenterFreq,
		AveFrameCoarseAcq,
		AveFramefineAcq,
		AveFrameCoarseTrq,
		AveFramefineTrq;

	S16	AutoReacq,
		TracklockSel,
		Zigzag,
		StepSize;
}FE_DVBS2_ReacquireParams_t;


u32 FE_DVBS2_GetModCod(STCHIP_Handle_t hChip);

S32 FE_DVBS2_GetUWPEsNo(STCHIP_Handle_t hChip, S32 Quant);

u32 FE_DVBS2_GetSymbolRate(STCHIP_Handle_t hChip, u32 MasterClock);

FE_DVBS2_State FE_DVBS2_GetState(STCHIP_Handle_t hChip, int Timeout);

void FE_DVBS2_CSMInitialize(STCHIP_Handle_t hChip, int Pilots, FE_DVBS2_ModCod_t ModCode, u32 SymbolRate, u32 MasterClock);

void FE_DVBS2_InitialCalculations(STCHIP_Handle_t hChip, FE_STB0899_DVBS2_InitParams_t *InitParams);

void FE_DVBS2_Reacquire(STCHIP_Handle_t hChip, FE_DVBS2_ReacquireParams_t * ReacquireParams);

long FE_DVBS2_CarrierWidth(long SymbolRate, FE_DVBS2_RRCAlpha_t Aplha);


#endif



