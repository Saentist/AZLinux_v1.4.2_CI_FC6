#ifndef STB0899_TUNER_H
#define STB0899_TUNER_H
	
#include "stb0899_chip.h"
	
/* RFMagic STB6100 tuner definition */

/*	LD	*/
#define RSTB6100_LD		0
#define FSTB6100_LD		0

/*	VCO	*/
#define RSTB6100_VCO		1
#define FSTB6100_OSCH		1
#define FSTB6100_OCK		2
#define FSTB6100_ODIV		3
#define FSTB6100_OSM		4

/*	NI	*/
#define RSTB6100_NI		2
#define FSTB6100_NI		5

/*	NF_LSB	*/
#define RSTB6100_NF_LSB		3
#define FSTB6100_NF_LSB		6

/*	K	*/
#define RSTB6100_K		4
#define FSTB6100_K		7
#define FSTB6100_PSD2		8
#define FSTB6100_NF_MSB		9

/*	G	*/
#define RSTB6100_G		5
#define FSTB6100_GCT		10
#define FSTB6100_G		11

/*	F	*/
#define RSTB6100_F		6
#define FSTB6100_F		12

/*	DLB	*/
#define RSTB6100_DLB		7
#define FSTB6100_DLB		13

/*	TEST1	*/
#define RSTB6100_TEST1		8
#define FSTB6100_TEST1		14

/*	TEST2	*/
#define RSTB6100_TEST2		9
#define FSTB6100_FCCK		15

/*	LPEN	*/
#define RSTB6100_LPEN		10
#define FSTB6100_BEN		16
#define FSTB6100_OSCP		17
#define FSTB6100_SYNP		18
#define FSTB6100_LPEN		19

/*	TEST3	*/
#define RSTB6100_TEST3		11
#define FSTB6100_TEST3		20

#define STB6100_NBREGS		12
#define STB6100_NBFIELDS	21
	
typedef enum
{
	TUNER_NO_ERR = 0,
	TUNER_TYPE_ERR,
	TUNER_ACK_ERR
} TUNER_Error_t;
		
typedef enum
{
	TUNER_IQ_NORMAL = 1,
	TUNER_IQ_INVERT = -1
}
TUNER_IQ_t;

typedef struct
{
	STCHIP_Info_t *Chip;   /* pointer to parameters to pass to the CHIP API */
	/* number of default values (must match number of registers) */
	u32           NbDefVal; 
       	u32           *DefVal;   /* pointer to table of default values */
       	u32           Reference; /* reference frequency (Hz) */ 
       	u32           IF;        /* Intermediate frequency (KHz) */
       	TUNER_IQ_t    IQ_Wiring; /* hardware I/Q invertion */
} 
TUNER_InitParams_t;

typedef TUNER_InitParams_t *TUNER_Handle_t;

/* functions --------------------------------------------------------------- */

TUNER_Handle_t STTunerInit(TUNER_InitParams_t *InitParams);

TUNER_Error_t TunerSetFrequency(TUNER_Handle_t hTuner,u32 Frequency);

u32 TunerGetFrequency(TUNER_Handle_t hTuner);

TUNER_Error_t TunerSetBandwidth(TUNER_Handle_t hTuner,u32 Bandwidth);

u32 TunerGetBandwidth(TUNER_Handle_t hTuner);

TUNER_Error_t TunerSetGain(TUNER_Handle_t hTuner,u32 Gain);

BOOL TunerGetStatus(TUNER_Handle_t hTuner);

TUNER_Error_t TunerWrite(TUNER_Handle_t hTuner);

TUNER_Error_t TunerRead(TUNER_Handle_t hTuner);


#endif

