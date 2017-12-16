
#include "linux/string.h"

#include "stb0899_tuner.h"
#include "stb0899_util.h"
#include "stb0899_chip.h"

static u32 STB6K_LOOKUP[11][3] = {
			/* low        high	        osm */
			{950000,	1000000,	0xA},
			{1000000,	1075000,	0xC},
			{1075000,	1200000,	0x0},
			{1200000,	1300000,	0x1},
			{1300000,	1370000,	0x2},
			{1370000,	1470000,	0x4},
			{1470000,	1530000,	0x5},
			{1530000,	1650000,	0x6},
			{1650000,	1800000,	0x8},
			{1800000,	1950000,	0xA},
			{1950000,	2150000,	0xC}
};

TUNER_Handle_t STTunerInit(TUNER_InitParams_t *hTunerInitParams)
{
	TUNER_Handle_t hTuner = NULL;
    	u32	*DefVal;

    	hTuner = (TUNER_Handle_t)kmalloc(sizeof(TUNER_InitParams_t), GFP_KERNEL);

   	if((hTuner != NULL) && (hTunerInitParams != NULL))
    	{
        	hTuner->NbDefVal	= hTunerInitParams->NbDefVal; 
        	hTuner->DefVal		= hTunerInitParams->DefVal;   
 		hTuner->Reference = 27000000;	/* 20 MHz reference */
    		hTuner->IF = 0; 	/* 0 MHz intermediate frequency */
    		hTuner->IQ_Wiring = TUNER_IQ_NORMAL;   /* No hardware IQ invertion */
	
		/* fill elements of external chip data structure */
		hTunerInitParams->Chip->NbRegs   = STB6100_NBREGS;
		hTunerInitParams->Chip->NbFields = STB6100_NBFIELDS;
		hTunerInitParams->Chip->ChipMode = STCHIP_MODE_NO_R_SUBADR;
		hTunerInitParams->Chip->WrStart  = RSTB6100_VCO;
		hTunerInitParams->Chip->WrSize   = 11;
		hTunerInitParams->Chip->RdStart  = RSTB6100_LD;
		hTunerInitParams->Chip->RdSize   = 12;
 
		if(hTunerInitParams->NbDefVal == STB6100_NBREGS)
		{
			hTuner->Chip = ChipOpen(hTunerInitParams->Chip);

			if(hTuner->Chip != NULL)
			{
				DefVal = hTuner->DefVal;
				/*REGISTER INITIALISATION	*/
	
				/*	LD	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_LD,"LD",0x0000,*DefVal++,STCHIP_ACCESS_R,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_LD,FSTB6100_LD,"LD",0,1,CHIP_UNSIGNED);

				/*	VCO	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_VCO,"VCO",0x0001,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_VCO,FSTB6100_OSCH,"OSCH",7,1,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_VCO,FSTB6100_OCK,"OCK",5,2,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_VCO,FSTB6100_ODIV,"ODIV",4,1,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_VCO,FSTB6100_OSM,"OSM",0,4,CHIP_UNSIGNED);

				/*	NI	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_NI,"NI",0x0002,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_NI,FSTB6100_NI,"NI",0,8,CHIP_UNSIGNED);

				/*	NF_LSB	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_NF_LSB,"NF_LSB",0x0003,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_NF_LSB,FSTB6100_NF_LSB,"NF_LSB",0,8,CHIP_UNSIGNED);

				/*	K	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_K,"K",0x0004,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_K,FSTB6100_K,"K",6,2,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_K,FSTB6100_PSD2,"PSD2",2,1,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_K,FSTB6100_NF_MSB,"NF_MSB",0,2,CHIP_UNSIGNED);

				/*	G	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_G,"G",0x0005,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_G,FSTB6100_GCT,"GCT",5,3,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_G,FSTB6100_G,"G",0,4,CHIP_UNSIGNED);

				/*	F	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_F,"F",0x0006,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_F,FSTB6100_F,"F",0,5,CHIP_UNSIGNED);

				/*	DLB	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_DLB,"DLB",0x0007,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_DLB,FSTB6100_DLB,"DLB",3,3,CHIP_UNSIGNED);

				/*	TEST1	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_TEST1,"TEST1",0x0008,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_TEST1,FSTB6100_TEST1,"TEST1",0,8,CHIP_UNSIGNED);

				/*	TEST2	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_TEST2,"TEST2",0x0009,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_TEST2,FSTB6100_FCCK,"FCCK",6,1,CHIP_UNSIGNED);

				/*	LPEN	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_LPEN,"LPEN",0x000a,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_LPEN,FSTB6100_BEN,"BEN",7,1,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_LPEN,FSTB6100_OSCP,"OSCP",6,1,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_LPEN,FSTB6100_SYNP,"SYNP",5,1,CHIP_UNSIGNED);
				ChipAddField(hTuner->Chip,RSTB6100_LPEN,FSTB6100_LPEN,"LPEN",4,1,CHIP_UNSIGNED);

				/*	TEST3	*/
				ChipAddReg(hTuner->Chip,STCHIP_REG_8,RSTB6100_TEST3,"TEST3",0x000b,*DefVal++,STCHIP_ACCESS_WR,STCHIP_NOT_POINTED,0,0);
				ChipAddField(hTuner->Chip,RSTB6100_TEST3,FSTB6100_TEST3,"TEST3",0,8,CHIP_UNSIGNED);
			}
		}	
		TunerWrite(hTuner);
	}
	return hTuner;
}

TUNER_Error_t TunerSetFrequency(TUNER_Handle_t hTuner, u32 Frequency)
{
	TUNER_Error_t error = TUNER_NO_ERR;
	u32 frequency;
	u32 stepsize;
	u32 nbsteps;
	u32 divider;
	u32 swallow;
	u8 vco,num;

	if(hTuner && !hTuner->Chip->ChipError)
	{		
		ChipSetFieldImage(hTuner->Chip,FSTB6100_ODIV,Frequency <= 1075000);
		if((Frequency<=1075000)||(Frequency>=1326000))
			ChipSetFieldImage(hTuner->Chip,FSTB6100_PSD2,1);
		else
			ChipSetFieldImage(hTuner->Chip,FSTB6100_PSD2,0);			
				
		num=0;
		while(!INRANGE(STB6K_LOOKUP[num][0],Frequency,STB6K_LOOKUP[num][1])&& (num<10)) num++;
		if(Frequency<950000)
			vco = (u8)(STB6K_LOOKUP[0][2]);
		else if(Frequency>2150000)
			vco = (u8)(STB6K_LOOKUP[10][2]);
		else
			vco = (u8)(STB6K_LOOKUP[num][2]);
				
		ChipSetFieldImage(hTuner->Chip,FSTB6100_OSM,vco);
		
		/*Flo=2*Ftuner*(ODIV+1)*/
		frequency=2*Frequency*(ChipGetFieldImage(hTuner->Chip,FSTB6100_ODIV)+1); 
				
		hTuner->Reference/=1000; /*Refrence in Khz*/
		/*flo=Fxtal*(DIV2+1)*(Ni+Nf/2^9)*/  
				
		/*Ni = floor (fVCO / (fXTAL * P))*/
		divider=(frequency/hTuner->Reference)/(ChipGetFieldImage(hTuner->Chip,FSTB6100_PSD2)+1);
				
		/*round ((fVCO / (fXTAL * P) - Ni) * 2^9)*/
		stepsize=frequency-divider*(ChipGetFieldImage(hTuner->Chip,FSTB6100_PSD2)+1)*(hTuner->Reference); /**/
		nbsteps=(stepsize*PowOf2(9))/((ChipGetFieldImage(hTuner->Chip,FSTB6100_PSD2)+1)*(hTuner->Reference));					  
		swallow=nbsteps; 
				
		ChipSetFieldImage(hTuner->Chip,FSTB6100_NI,divider);
		ChipSetFieldImage(hTuner->Chip,FSTB6100_NF_LSB,(swallow&0xFF));
		ChipSetFieldImage(hTuner->Chip,FSTB6100_NF_MSB,(swallow>>8));
		hTuner->Reference*=1000; 
		ChipSetFieldImage(hTuner->Chip,FSTB6100_LPEN,0);/* PLL loop disabled */
		ChipSetFieldImage(hTuner->Chip,FSTB6100_OSCH,1);/* VCO search enabled */
		ChipSetFieldImage(hTuner->Chip,FSTB6100_OCK,0); /* VCO fast search*/
		error = TunerWrite(hTuner);

				
		ChipSetFieldImage(hTuner->Chip,FSTB6100_LPEN,1); /* PLL loop enabled */
		error = TunerWrite(hTuner);
				
		WAIT_N_MS(10);
		ChipSetFieldImage(hTuner->Chip,FSTB6100_OSCH,0); /* VCO search disabled */
		ChipSetFieldImage(hTuner->Chip,FSTB6100_OCK,3);  /* VCO search clock off */
		ChipSetFieldImage(hTuner->Chip,FSTB6100_FCCK,0); /*LPF BW setting clock disabled */
		error = TunerWrite(hTuner);
	}
		
	return error;
}

u32 TunerGetFrequency(TUNER_Handle_t hTuner)
{
	u32 frequency = 0;
	u32 divider = 0;
	u32 swallow;
	u32 psd2;
				
	if(hTuner && !hTuner->Chip->ChipError)	
	{
		TunerRead(hTuner);
		swallow=(ChipGetFieldImage(hTuner->Chip,FSTB6100_NF_MSB)<<8)  /*Nf val*/
				+ChipGetFieldImage(hTuner->Chip,FSTB6100_NF_LSB);
				
		/*Ni = floor (fVCO / (fXTAL * P))*/
		divider=ChipGetFieldImage(hTuner->Chip,FSTB6100_NI); /*NI val*/
		psd2=ChipGetFieldImage(hTuner->Chip,FSTB6100_PSD2);
				
		frequency=(((1+psd2)*(hTuner->Reference/1000)*swallow)/512);
		frequency+=(hTuner->Reference/1000) * (divider)*(1+psd2);
		/*Flo=Fxtal*P*(Ni+Nf/2^9) . P=DIV2+1 */
				
		frequency=frequency/((ChipGetFieldImage(hTuner->Chip,FSTB6100_ODIV)+1)*2);			
	}
	
	return frequency;
}


TUNER_Error_t TunerSetBandwidth(TUNER_Handle_t hTuner,u32 Bandwidth)
{
	TUNER_Error_t error = TUNER_NO_ERR; 
	u8 val;
	
	if(hTuner)	
	{
		if((Bandwidth/2) > 36000000)   /*F[4:0] BW/2 max =31+5=36 mhz for F=31*/
			val = 31;
		else if((Bandwidth/2) < 5000000) /* BW/2 min = 5Mhz for F=0 */
			val = 0;
		else							 /*if 5 < BW/2 < 36*/
			val = (u8)((Bandwidth/2)/1000000 - 5);
				
		ChipSetFieldImage(hTuner->Chip,FSTB6100_FCCK,(hTuner->Reference/1000000)); /* FCL_Clk=FXTAL/FCL=1Mhz */
		TunerWrite(hTuner);
		ChipSetFieldImage(hTuner->Chip,FSTB6100_F,val);
		error = TunerWrite(hTuner);  
		ChipSetFieldImage(hTuner->Chip,FSTB6100_FCCK,0); /*FCL turned off*/
		TunerWrite(hTuner);		
	}
	
	return error;
}

u32 TunerGetBandwidth(TUNER_Handle_t hTuner)
{
	u32 bandwidth = 0;
	u8 val = 0;
	
	if(hTuner)	
	{
		TunerRead(hTuner);
		val = (u8)ChipGetFieldImage(hTuner->Chip,FSTB6100_F);
	
		bandwidth = (val+5)*2000000;	/* x2 for ZIF tuner BW/2=F+5 Mhz*/
	}
	
	return bandwidth;
}

TUNER_Error_t TunerSetGain(TUNER_Handle_t hTuner,u32 Gain)
{
	TUNER_Error_t error = TUNER_NO_ERR;
		
	if(hTuner)	
	{
		//printk(KERN_ERR "TunerSetGain\n");
		ChipSetFieldImage(hTuner->Chip,FSTB6100_G,(Gain/2)+7);
		error = TunerWrite(hTuner);	
	}	
	return error;
}

BOOL TunerGetStatus(TUNER_Handle_t hTuner)
{
	BOOL locked = FALSE;
	
	if(hTuner)	
	{
		TunerRead(hTuner);
	
		if(!hTuner->Chip->ChipError)
		{
			TunerRead(hTuner);
			locked = ChipGetFieldImage(hTuner->Chip,FSTB6100_LD);
		}	
	}
	
	return locked;
}

TUNER_Error_t TunerWrite(TUNER_Handle_t hTuner)
{
	TUNER_Error_t error = TUNER_NO_ERR;
	
	if(hTuner)	
	{
		unsigned char data[3];
		
		//repeater
		data[0] = 0xf1;
		data[1] = 0x2a;
		data[2] = 0x5c | 0x80;
		I2cWrite(hTuner->Chip, data, 3);
		msleep(100);
	
		ChipSetRegisters(hTuner->Chip,RSTB6100_VCO,11);	
	}
	
	return error;
}

TUNER_Error_t TunerRead(TUNER_Handle_t hTuner)
{
	TUNER_Error_t error = TUNER_NO_ERR;
	
	if(hTuner)	
	{
		unsigned char data[3];

		//repeater
		data[0] = 0xf1;
		data[1] = 0x2a;
		data[2] = 0x5c | 0x80;
		I2cWrite(hTuner->Chip, data, 3);
		msleep(100);
							
		ChipGetRegisters(hTuner->Chip, RSTB6100_LD, 12);
	}
	return error;
}






