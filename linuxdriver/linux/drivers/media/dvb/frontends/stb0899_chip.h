/* -------------------------------------------------------------------------
File Name: stn0899_chip.h

Description: Present a register based interface to hardware connected on an I2C bus.

Copyright (C) 1999-2001 STMicroelectronics

History:
  date: 10-October-2001
version: 1.0.0
 author: SA
comment: STAPIfied by GP

---------------------------------------------------------------------------- */

/* define to prevent recursive inclusion */
#ifndef STB0899_CHIP_H
#define STB0899_CHIP_H

#include "stb0899_common.h"
#include <linux/dvb/frontend.h>
#include "dvb_frontend.h"


/* enumerations------------------------------------------------------------- */

/* access modes for fields and registers */
typedef enum
{
    STCHIP_ACCESS_WR,  /* can be read and written */
    STCHIP_ACCESS_R,   /* only be read from */
    STCHIP_ACCESS_W,   /* only be written to */
    STCHIP_ACCESS_NON  /* cannot be read or written (guarded register, e.g. register skipped by ChipApplyDefaultValues() etc.) */
}
STCHIP_Access_t;

/* register field type */
typedef enum
{
    CHIP_UNSIGNED,
    CHIP_SIGNED
}
STCHIP_FieldType_t;

/* error codes */
typedef enum
{
    CHIPERR_NO_ERROR = 0,       /* 0    No error encountered */
    CHIPERR_INVALID_HANDLE,     /* 1    Using of an invalid chip handle */
    CHIPERR_INVALID_REG_ID,     /* 2    Using of an invalid register */
    CHIPERR_INVALID_FIELD_ID,   /* 3    Using of an Invalid field */
    CHIPERR_INVALID_FIELD_SIZE, /* 4    Using of a field with an invalid size */
    CHIPERR_I2C_NO_ACK,         /* 5    No acknowledge from the chip */
    CHIPERR_I2C_BURST           /* 6    Two many registers accessed in burst mode */
}
STCHIP_Error_t;

/* how to access I2C bus */
typedef enum
{
    STCHIP_MODE_SUBADR_8,       /* <addr><reg8><data><data> (e.g. demod chip) */
    STCHIP_MODE_SUBADR_16,      /* <addr><regM8><regL8><data><data> (e.g. demod chip) */
    STCHIP_MODE_NOSUBADR,       /* <addr><data>|<data><data><data> (e.g. tuner chip) */
    STCHIP_MODE_NO_R_SUBADR
}
STCHIP_Mode_t;

typedef enum
{
	STCHIP_REG_8,
	STCHIP_REG_16,
	STCHIP_REG_24,
	STCHIP_REG_32
}STCHIP_RegSize_t;

typedef enum
{
	STCHIP_NOT_POINTED=0,
	STCHIP_POINTED
}STCHIP_Pointed_t;

/* structures ----------------------------
---------------------------------- */

/* register information */
typedef struct
{
	u16             		Addr;  		/* Address */
    	STCHIP_RegSize_t 	Size;	  	/* Register size in Byte*/
    	u32   			Default;		/* Default value */
    	u32	 		Value;		/* Current value */
    	char           	 	Name[30];		/* Name */
    	STCHIP_Access_t 		Access;		/* access mode */
    	STCHIP_Pointed_t 	Pointed;		/* Register Pointed or not*/
    	u16 			PointerRegAddr;	/* Pointer Adress*/
    	u32 			BaseAdress;	/* Base Adress*/
}
STCHIP_Register_t;

/* register field information */
typedef struct
{
    u16                	Reg;      /* Register index */
    unsigned char      	Pos;      /* Bit position */
    unsigned char      	Bits;     /* Bit width */
    u32			Mask;	  /* Mask compute with width and position */
    STCHIP_FieldType_t 	Type;     /* Signed or unsigned */
    char               	Name[30]; /* Name */
}
STCHIP_Field_t;

/* data about a specific chip */
typedef struct stchip_Info_t
{
    	unsigned char       I2cAddr;          /* Chip I2C address */
    	struct i2c_adapter  *I2c_adap;
    	char                Name[30];         /* Name of the chip */
    	int                 NbRegs;           /* Number of registers in the chip */
    	int                 NbFields;         /* Number of fields in the chip */
    	STCHIP_Register_t   *pRegMap;         /* Pointer to register map */
    	STCHIP_Field_t      *pFieldMap;       /* Pointer to field map */
    	STCHIP_Error_t      ChipError;        /* Error state */
    	STCHIP_Mode_t       ChipMode;         /* Access bus in demod (SubAdr) or tuner (NoSubAdr) mode */

#ifdef HOST_PC				  /* PC specific parameters */
	BOOL		Repeater;         /* Is repeater enabled or not ? */
	/* Owner of the repeater */
	struct stchip_Info_t *RepeaterHost;
	/* Pointer to repeater routine */
	STCHIP_Error_t	(*RepeaterFn)(struct stchip_Info_t *hChip,BOOL State);
	
	/* Parameters needed for non sub address devices */
	u32		WrStart;	/* Id of the first writable register */
	u32		WrSize;         /* Number of writable registers */
	u32		RdStart;	/* Id of the first readable register */
	u32		RdSize;		/* Number of readable registers */
#endif
}
STCHIP_Info_t;

/* Handle to a chip */
typedef STCHIP_Info_t *STCHIP_Handle_t;

/* Pointer to repeater routine */ 
typedef STCHIP_Error_t  (*STCHIP_RepeaterFn_t)(STCHIP_Handle_t hChip,BOOL State);

/* functions --------------------------------------------------------------- */

/* Creation and destruction routines */
STCHIP_Handle_t ChipOpen(STCHIP_Info_t *hChipOpenParams);

STCHIP_Error_t ChipAddReg(STCHIP_Handle_t hChip, STCHIP_RegSize_t Size, u16 RegId, char * Name, u16 Address, u32 Default, STCHIP_Access_t Access, STCHIP_Pointed_t Pointed, u16 PointerRegAddr, u32 BaseAdress);

STCHIP_Error_t ChipAddField(STCHIP_Handle_t hChip, u16 RegId, u32 FieldId, char *Name, char Pos, char NbBits, STCHIP_FieldType_t Type);

/* Utility routines */
STCHIP_Error_t ChipApplyDefaultValues(STCHIP_Handle_t hChip);

/* Access routines */
int I2cWrite(STCHIP_Handle_t hChip, u8 *Data, u8 NbData);
int I2cRead(STCHIP_Handle_t hChip, u8 *Data, u8 NbData);

STCHIP_Error_t ChipSetOneRegister(STCHIP_Handle_t hChip, u16 RegId, u32 Data);
int	       ChipGetOneRegister(STCHIP_Handle_t hChip, u16 RegId);

STCHIP_Error_t ChipSetRegisters(STCHIP_Handle_t hChip, int FirstReg, int NbRegs);
STCHIP_Error_t ChipGetRegisters(STCHIP_Handle_t hChip, int FirstReg, int NbRegs);

STCHIP_Error_t ChipSetField(STCHIP_Handle_t hChip, u32 FieldId, int Value);
int            ChipGetField(STCHIP_Handle_t hChip, u32 FieldId);

STCHIP_Error_t ChipSetFieldImage(STCHIP_Handle_t hChip, u32 FieldId, int Value);
int            ChipGetFieldImage(STCHIP_Handle_t hChip, u32 FieldId);



#endif



