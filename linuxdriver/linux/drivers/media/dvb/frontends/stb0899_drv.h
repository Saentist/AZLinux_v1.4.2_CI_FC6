#ifndef STB0899_DRV_H
#define STB0899_DRV_H

#include "stb0899.h"
#include "stb0899_common.h"
#include "stb0899_dvbs2util.h"
#include "stb0899_util.h"
#include "stb0899_init.h"
#include "stb0899_tuner.h"

/****************************************************************
		COMMON STRUCTURES AND TYPEDEF
****************************************************************/		
typedef enum
{
	FE_NO_ERROR,
	FE_INVALID_HANDLE,
	FE_ALLOCATION,
	FE_BAD_PARAMETER,
	FE_ALREADY_INITIALIZED,
	FE_I2C_ERROR,
	FE_SEARCH_FAILED,
	FE_TRACKING_FAILED,
	FE_TERM_FAILED
} FE_STB0899_Error_t;
	
typedef enum
{
	FE_MOD_BPSK,
	FE_MOD_QPSK,
	FE_MOD_OQPSK,
	FE_MOD_8PSK
} FE_STB0899_Modulation_t;
	
typedef enum
{
	FE_IQ_AUTO,
	FE_IQ_NORMAL,
	FE_IQ_SWAPPED
}FE_STB0899_IQ_Inversion;

typedef enum
{
	FE_1_2 =13,
	FE_2_3 =18,	
	FE_3_4 =21,
	FE_5_6 =24,
	FE_6_7 =25,
	FE_7_8 =26
}FE_STB0899_Rate_t;
	
typedef enum
{	
	FE_PARALLEL_CLOCK,
	FE_SERIAL_MASTER_CLOCK,
	FE_SERIAL_VCODIV6_CLOCK
} FE_STB0899_Clock_t;

typedef enum
{
	FE_PARITY_ON,
	FE_PARITY_OFF
} FE_STB0899_DataParity_t;
/* The FEC mode corresponds to the DVB standard	*/
typedef enum
{	
	FE_DVBS1_STANDARD,	
	FE_DVBS2_STANDARD,
	FE_DSS_STANDARD
} FE_STB0899_CodingStandard_t;

/*Internal error definitions*/
typedef enum 
{
	FE_IERR_NO,			/*no error	*/
	FE_IERR_I2C,			/*I2C error	*/
	FE_IERR_ZERODIV,		/*division by zero*/
	FE_IERR_PARAM,			/*wrong parameters*/
	FE_IERR_UNKNOWN			/*unknown error	*/
} FE_STB0899_ErrorType_t;

typedef enum
{
	FE_LOC_NOWHERE,			/*no location	*/
	FE_LOC_SRHINIT,			/*in SearchInit	*/
	FE_LOC_SRHRUN,			/*in SearchRun	*/
	FE_LOC_SRHTERM,			/*in SearchTerm	*/
	FE_LOC_SETSR,			/*in SetSymbolRate*/
	FE_LOC_TIMTCST,			/*in TimingTimeConstant	*/
	FE_LOC_DERTCST,			/*in DerotTimeConstant*/
	FE_LOC_DATTCST,			/*in DataTimeConstant*/
	FE_LOC_CHKTIM,			/*in CheckTiming*/
	FE_LOC_SRHCAR,			/*in SearchCarrier*/
	FE_LOC_SRHDAT,			/*in SearchData	*/
	FE_LOC_CHKRNG,			/*in CheckRange	*/
	FE_LOC_SELLPF			/*in SelectLPF	*/
} FE_STB0899_Location_t;

/****************************************************************
		INIT STRUCTURES
	structure passed to the FE_STB0899_Init() function  
****************************************************************/
 typedef struct
{
	STB0899_InitParams_t *STB0899Init;  /* parameters to pass to initialize the STB0899 */
	TUNER_InitParams_t   *TunerInit;    /* parameters to pass to initialize the Tuner*/
	STCHIP_Info_t        *LnbInit;      /* parameters to pass to initialize the LNBP21*/
        FE_STB0899_CodingStandard_t Standard; 	/* standard used : DVBS1,DVBS2*/
        FE_STB0899_Clock_t 	    Clock;    	/* Clock settings */
        FE_STB0899_DataParity_t     Parity;   	/* parity of the data*/
} FE_STB0899_InitParams_t;

/****************************************************************
		SEARCH STRUCTURES
 ****************************************************************/
typedef struct
{
	u32 Frequency;		/* transponder frequency (in KHz)*/
	u32 SymbolRate; 	/* transponder symbol rate  (in bds)*/
	u32 SearchRange;	/* range of the search (in Hz) */
	FE_STB0899_Modulation_t Modulation;	/* modulation*/
	FE_STB0899_CodingStandard_t Standard;	/*Dvb dvbs2*/
	FE_STB0899_IQ_Inversion IQ_Inversion;	/* IQ spectrum search for DVBS2*/
} FE_STB0899_SearchParams_t;
	
typedef struct
{
	BOOL Locked;			/* Transponder found */
	u32 Frequency;			/* found frequency*/
	u32 SymbolRate;			/* founded symbol rate*/
	FE_STB0899_Rate_t Rate;		/* puncture rate  for DVBS1*/
	FE_DVBS2_ModCod_t ModCode;	/* found modecode only for DVBS2*/
	BOOL Pilots;			/* pilots found*/
	FE_DVBS2_FRAME FrameLength;	/* found frame length*/
} FE_STB0899_SearchResult_t;
	
/***********************************************************
		INFO STRUCTURE
***********************************************************/
typedef struct
{
	BOOL Locked;				/* Transponder locked*/
	u32 Frequency;				/* transponder frequency (in KHz)*/
	u32 SymbolRate;				/* transponder symbol rate  (in Mbds)*/
	FE_STB0899_Modulation_t Modulation;	/* modulation*/
	FE_STB0899_Rate_t Rate;			/* puncture rate for DVBS1 mode	*/
	FE_DVBS2_ModCod_t ModCode;		/* only for DVBS2*/ 
	BOOL Pilots;				/* Pilots on/off only for DVB-S2*/
	FE_DVBS2_FRAME FrameLength;		/* found frame length*/
	S32 Power;				/* Power of the RF signal (dBm)*/	
	u32 C_N;				/* Carrier to noise ratio*/
	u32 BER;				/* Bit error rate*/
	S16 SpectralInv;			/* I,Q Inversion */	
} FE_STB0899_SignalInfo_t;

typedef struct
{
	FE_STB0899_ErrorType_t Type;	 /* Error type	*/
	FE_STB0899_Location_t Location;	 /* Error location*/
} FE_STB0899_InternalError_t;

typedef struct
{	
	/*DVB Internal Params*/
	u32 Frequency;				/*Transponder frequency (KHz)*/
	FE_STB0899_SIGNALTYPE_t	SignalType;	/*Type of founded signal*/
	FE_STB0899_Rate_t PunctureRate;		/*Puncture rate found*/
	u32 SymbolRate; 			/*Symbol rate (Bds)*/
	
	/*DVBS2 Internal Params*/
	FE_DVBS2_State	DVBS2SignalType;
	u32 DVBS2SymbolRate; 			/*founded Symbol rate (Bds)*/
	FE_DVBS2_ModCod_t ModCode;		/*founded ModCod*/
	BOOL Pilots;				/*Pilots founded*/
	FE_DVBS2_FRAME FrameLength;		/* found frame length*/
} FE_STB0899_InternalResults_t;

/*Internal param structure*/
typedef struct
{
	STCHIP_Handle_t hDemod;		/*Handle to the chip*/
	TUNER_Handle_t	hTuner;		/*Handle to the tuner*/
	STCHIP_Handle_t hLnb;		/*Handle to the chip*/

	FE_STB0899_CodingStandard_t Standard;
	S32	Quartz;			/*Quartz frequency (Hz) */
	S32	Frequency,		/*Current tuner frequency (KHz) */
		BaseFreq,		/*Start tuner frequency (KHz) */
		SubRange,		/*Current sub range (Hz) */
		TunerStep,		/*Tuner step (Hz) */
		TunerOffset,		/*Tuner offset relative to the carrier (Hz) */
		TunerBW;		/*Current bandwidth of the tuner (Hz) */ 

	/*DVBS1 Params*/
	FE_STB0899_SIGNALTYPE_t State;	/*Current state of the search algorithm */
	FE_DVBS2_State	DVBS2State;

	S32	SymbolRate,		/*Symbol rate (Bds) */
		MasterClock,		/*Master clock frequency (Hz) */
		Mclk,			/*Divider factor for masterclock (binary value) */
		SearchRange,		/*Search range (Hz) */
		RollOff;		/*Current RollOff of the filter (x100) */
		
	S16	DerotFreq,		/*Current frequency of the derotator (Hz) */
		DerotPercent,		/*Derotator step (in thousands of symbol rate) */
		DerotStep,		/*Derotator step (binary value) */
		Direction,		/*Current search direction */ 
		Tagc1,			/*Agc1 time constant (ms) */
		Tagc2,			/*Agc2 time constant (ms) */
		Ttiming,		/*Timing loop time constant (ms) */
		Tderot,			/*Derotator time constant (ms) */
		Tdata,			/*Data recovery time constant (ms) */
		SubDir;			/*Direction of the next sub range */
	
	/*DVBS2 Params*/
	S32 	DVBS2SymbolRate,	/*Symbol rate (Bds) */	
		AgcGain,		/* RF AGC Gain */
		AveFrameCoarse,
		AveFramefine,
		AgcThreshold,
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
	
	FE_STB0899_IQ_Inversion SpectralInv; 
	FE_DVBS2_Mode_t mod;
	FE_DVBS2_AcqMode AcqMode;
	FE_DVBS2_RRCAlpha_t RrcAlpha;

	/*Result and error */
	FE_STB0899_InternalResults_t	Results;	/* Results of the search*/
	FE_STB0899_InternalError_t	Inl_Error;	/* Last error encountered*/
}FE_STB0899_InternalParams_t;

/****************************************************************
		API FUNCTIONS
****************************************************************/

FE_STB0899_Handle_t FE_STB0899_Init(FE_STB0899_InitParams_t *pInit);

FE_STB0899_Error_t FE_STB0899_Search(FE_STB0899_Handle_t Handle, FE_STB0899_SearchParams_t * pParams, FE_STB0899_SearchResult_t * pResult);

FE_STB0899_Error_t FE_STB0899_GetSignalInfo(FE_STB0899_Handle_t Handle, FE_STB0899_SignalInfo_t * pInfo);

FE_STB0899_Error_t FE_STB0899_DiseqcSend(FE_STB0899_Handle_t Handle, u8 *Data, u32 NbData);

FE_STB0899_Error_t FE_STB0899_Set22KHZContinues(FE_STB0899_Handle_t Handle, BOOL ToneOn);

FE_STB0899_Error_t FE_STB0899_Term(FE_STB0899_Handle_t Handle);

FE_STB0899_Error_t FE_STB0899_SetMclk(FE_STB0899_Handle_t Handle, u32 Mclk, u32 ExtClk);

#endif

