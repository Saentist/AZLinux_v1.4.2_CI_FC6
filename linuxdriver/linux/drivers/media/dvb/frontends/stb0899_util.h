#ifndef STB0899_UTIL_H
#define STB0899_UTIL_H

#include "stb0899_chip.h"


#define FE_STB0899_MAXLOOKUPSIZE 50

#define MULT32X32(a,b) (((((long)((a)>>16))*((long)((b)>>16)))<<1) +((((long)((a)>>16))*((long)(b&0x0000ffff)))>>15) + ((((long)((b)>>16))*((long)((a)&0x0000ffff)))>>15))


/*Signal type enum*/
typedef enum
{
	NOAGC1=0,
	AGC1OK,         //1
	NOTIMING,       //2
	ANALOGCARRIER,  //3
	TIMINGOK,       //4
	NOAGC2,         //5
	AGC2OK,         //6
	NOCARRIER,      //7
	CARRIEROK,      //8
	NODATA,         //9
	FALSELOCK,      //10
	DATAOK,         //11
	OUTOFRANGE,     //12
	RANGEOK         //13
} FE_STB0899_SIGNALTYPE_t;

/* Counter enum */
typedef enum
{
	COUNTER1 = 0,
	COUNTER2 = 1,
	COUNTER3 = 2
} ERRORCOUNTER;

/* One point of the lookup table */
typedef struct
{
	S32 realval;	/*real value */
	S32 regval;	/*binary value */
} FE_STB0899_LOOKPOINT_t;

/*Lookup table definition*/
typedef struct
{
	S32 size; /*Size of the lookup table*/
	FE_STB0899_LOOKPOINT_t table[FE_STB0899_MAXLOOKUPSIZE]; /*Lookup table*/
} FE_STB0899_LOOKUP_t;
	
typedef enum
{
	DVBS1_STANDARD,
	DVBS2_STANDARD,
	DSS_STANDARD
}STB0899_STANDARD;


long PowOf2(int number);
long Log2Int(int number);
long Log10Int(long logarg);

u32 FE_STB0899_GetMclkFreq(STCHIP_Handle_t hChip, u32 SyntFreq);
void FE_STB0899_SetStandard(STCHIP_Handle_t hChip, STB0899_STANDARD Standard);

u32 FE_STB0899_GetError(STCHIP_Handle_t hChip, u32 Standard);
u16 FE_STB0899_GetRFLevel(STCHIP_Handle_t hChip,FE_STB0899_LOOKUP_t *lookup,STB0899_STANDARD Standard);
u16 CarrierGetQuality(STCHIP_Handle_t hChip, FE_STB0899_LOOKUP_t *lookup, STB0899_STANDARD Standard);

STCHIP_Error_t STB0899_RepeaterFn(STCHIP_Handle_t hChip, BOOL State);


#endif

