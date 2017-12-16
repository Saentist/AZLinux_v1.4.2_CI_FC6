
#include "stb0899_util.h"
#include "stb0899_init.h"


static char l1d8[256] =	/* Lookup table to evaluate exponent*/
{
	8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 
     	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

short DSPnormalize(long *arg, short maxnorm)
{
	long input;
	int one, two, three, four;
	long  retval;
	input = (*arg<0)?~(*arg):*arg;

	one = ((unsigned char) (input >> 24));   
	two = ((unsigned char) (input >> 16));
	three = ((unsigned char)(input >> 8));
	four  = ((unsigned char)(input));

	retval = one ? l1d8[one]-1L  : two ? 7L+ l1d8[two] : three ? 15L + l1d8[three] : 23L + l1d8[four];

	return ((retval>maxnorm)? maxnorm:(short)(retval)); 
}

/*****************************************************
**FUNCTION	::	PowOf2
**ACTION	::	Compute  2^n (where n is an integer) 
**PARAMS IN	::	number -> n
**PARAMS OUT::	NONE
**RETURN	::	2^n
*****************************************************/
long PowOf2(int number)
{
	int i;
	long result = 1;
	
	for(i=0; i<number; i++)
		result *= 2;
		
	return result;
}

long Log2Int(int number)
{
	int i = 0;

	while(PowOf2(i) <= ABS(number))
		i++;

	if (number == 0)
		i = 1;

    	return i - 1;
}

long Log10Int(long logarg)
{
 	long powval1,powval2,powval3,powval4,powval5,powval6,powval7,powval8;
	long coeff0, coeff1, coeff2;
	long powexp_l;
	short powexp;
	int SignFlag = 0;
	
	/*Initialize coeffs 0.31 format*/
	coeff0	= 1422945213;		/* This coeff in 0.31 format;*/
	coeff1	= 2143609158;
	coeff2	= 724179374;

        /*Normalize power measure */
	powexp = DSPnormalize(&logarg, 32);
	logarg<<=powexp;
	
	if(powexp&0x8000)
	{
		SignFlag = 1;
		powexp	= -powexp;
	}
	
	powexp = 31 - powexp;
	powexp_l = ((long)(powexp)<<24);
	powval1 = powexp_l;
	powval4 = MULT32X32(logarg,coeff1);
	powval5 = coeff0 - powval4;
	powval2 = MULT32X32(logarg,logarg);
	powval3 = MULT32X32(powval2,coeff2);
	powval6 = powval5 + powval3;
	powval7 = powval6>>5;
	powval8 = powval1 - powval7;

	if(SignFlag ==1)
		powval8 = -powval8;
	
	return(powval8);	
}

void FE_STB0899_SetStandard(STCHIP_Handle_t hChip,STB0899_STANDARD Standard)
{
	switch(Standard)
	{
		case DVBS1_STANDARD:
			/* FEC mode */
			ChipSetField(hChip,FSTB0899_FECM_RESERVED0,0x00); /*DVB*/
			ChipSetField(hChip,FSTB0899_VITON,1); /*viterbi_on*/
			/* Reed-Solomon upper layer control */
			ChipSetOneRegister(hChip,RSTB0899_RSULC,0xb1);
			/* Transport Stream upper layer control */
			ChipSetOneRegister(hChip,RSTB0899_TSULC,0x40);
			/* Reed-Solomon lower layer control */
			ChipSetOneRegister(hChip,RSTB0899_RSLLC,0x42);
			/* Transport Stream lower layer control */
			ChipSetOneRegister(hChip,RSTB0899_TSLPL,0x12);
			
			ChipSetField(hChip,FSTB0899_FRESLDPC,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKH8PSK90,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKFEC90,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKFEC180,1);
			
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKPKDLIN90,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKPKDLIN180,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKINTBUF180,1);
			
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKCORE270,0);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKS2DMD90,1);
		break;
		
		case DVBS2_STANDARD:
			/* FEC mode */
			ChipSetField(hChip,FSTB0899_FECM_RESERVED0,0x00); /*DVB*/
			ChipSetField(hChip,FSTB0899_VITON,0); /*viterbi_off*/
			/* Reed-Solomon upper layer control */
			ChipSetOneRegister(hChip,RSTB0899_RSULC,0xb1);
			/* Transport Stream upper layer control */
			ChipSetOneRegister(hChip,RSTB0899_TSULC,0x42);
			/* Reed-Solomon lower layer control */
			ChipSetOneRegister(hChip,RSTB0899_RSLLC,0x40);
			/* Transport Stream lower layer control */
			ChipSetOneRegister(hChip,RSTB0899_TSLPL,0x2);
			
			ChipSetField(hChip,FSTB0899_FRESLDPC,0);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKH8PSK90,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKFEC90,0);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKFEC180,0);
			
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKPKDLIN90,0);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKPKDLIN180,0);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKINTBUF180,0);

			ChipSetFieldImage(hChip,FSTB0899_STOP_CKCORE270,0);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKS2DMD90,0);

		break;
		
		case DSS_STANDARD:
			/* FEC mode */
			ChipSetField(hChip,FSTB0899_FECM_RESERVED0,0x01); /*DIRECTV*/
			ChipSetField(hChip,FSTB0899_VITON,1); /*viterbi_on*/
			/* Reed-Solomon upper layer control */
			ChipSetOneRegister(hChip,RSTB0899_RSULC,0xa1);
			/* Transport Stream upper layer control */
			ChipSetOneRegister(hChip,RSTB0899_TSULC,0x61);
			/* Reed-Solomon lower layer control */
			ChipSetOneRegister(hChip,RSTB0899_RSLLC,0x42);

			ChipSetField(hChip,FSTB0899_FRESLDPC,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKH8PSK90,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKFEC90,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKFEC180,1);
			
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKPKDLIN90,1);
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKPKDLIN180,1);
			
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKCORE270,0); 
			ChipSetFieldImage(hChip,FSTB0899_STOP_CKS2DMD90,1);

		break;
		
	}
	ChipSetFieldImage(hChip,FSTB0899_STOP_CKADCI90,0);
	ChipSetRegisters(hChip,RSTB0899_STOPCLK1,2);
}

/*****************************************************
**FUNCTION	::	FE_STB0899_GetMclkFreq
**ACTION	::	Set the STB0899 master clock frequency
**PARAMS IN	::  	hChip==>handle to the chip
**			ExtClk==>External clock frequency (Hz)
**PARAMS OUT::	NONE
**RETURN	::	Synthesizer frequency (Hz)
*****************************************************/
u32 FE_STB0899_GetMclkFreq(STCHIP_Handle_t hChip, u32 ExtClk)
{
	u32 	mclk = 90000000,
		div = 0;
			
	div = ChipGetField(hChip, FSTB0899_MDIV);
	mclk = (div + 1) * ExtClk / 6;

	return mclk;
}


