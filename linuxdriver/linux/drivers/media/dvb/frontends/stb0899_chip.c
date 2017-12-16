
#include <linux/string.h>
#include <linux/i2c.h>

#include "dvb_frontend.h"
#include "stb0899_chip.h"

typedef struct node
{
	STCHIP_Handle_t hChip;
	struct node *pNextNode;
}NODE;

static NODE *pFirstNode = NULL;

static u32 LastBaseAdress=0xffffffff;
static u16 LastPointer=0xffff;

extern int debug;

/*******************************************************************************
 **FUNCTION  ::  I2cWrite
 **ACTION    ::  Write data to the slave
 **PARAMS IN ::  hChip	==> Handle to the chip
 **		 NbData	==> Number of data to write/read
 **		 Data	==> buffer containing data which will be writen
 **PARAMS OUT::	 NONE
 **RETURN    ::   if success return ret  else EREMOTEIO
 ********************************************************************************/
int I2cWrite(STCHIP_Handle_t hChip, u8 *Data, u8 NbData)
{
	int ret;

	struct i2c_msg msg = {
		.addr = hChip->I2cAddr,
		.flags = 0,
		.buf = Data,
		.len = NbData
		};

	ret = i2c_transfer(hChip->I2c_adap, &msg, 1);
	if (ret != 1) {
		dprintk("I2cWrite Error");
		return -EREMOTEIO;
	} else {
		//dprintk("I2cWrite successful\n");
		
		return ret;
	}
	return -EIO;
}

/*******************************************************************************
 **FUNCTION  ::  I2cRead
 **ACTION    ::  Read data from the slave
 **PARAMS IN ::  hChip	==> Handle to the chip
 **		 NbData	==> Number of data to write/read
 **PARAMS OUT::	 Data	==> Buffer containing data readed
 **RETURN    ::  if success return ret  else EREMOTEIO
 ********************************************************************************/
int I2cRead(STCHIP_Handle_t hChip, u8 *Data, u8 NbData)
{
	int ret;

	struct i2c_msg msg = {
		.addr = hChip->I2cAddr,
		.flags = I2C_M_RD,
		.buf = Data,
		.len = NbData
		};

	ret = i2c_transfer(hChip->I2c_adap, &msg, 1);
	if (ret != 1) {
		dprintk("I2cRead Error");
		return -EREMOTEIO;
	} else {
		//dprintk("I2cRead successful");
		return ret;
	}	
	return -EIO;
 }

/* List routines*/
static NODE *AppendNode(STCHIP_Handle_t hChip)
{
	NODE *pNode = pFirstNode;
	
	if(pNode == NULL)
	{   /* Allocation of the first node*/
		pNode = (NODE *)kmalloc(sizeof(NODE), GFP_KERNEL);
		pFirstNode = pNode;
	}
	else
	{   /* Allocation of a new node */
		while(pNode->pNextNode != NULL) /* Search of the last node*/
			pNode = pNode->pNextNode;
			
		/* Memory allocation */
		pNode->pNextNode = (NODE *)kmalloc(sizeof(NODE),GFP_KERNEL);
		
		if(pNode->pNextNode != NULL) /* Check allocation */
			pNode = pNode->pNextNode;
		else
			pNode = NULL;
	}
	
	if(pNode != NULL) /* if allocation ok */
	{
		/* Fill the node */
		pNode->hChip = hChip;
		pNode->pNextNode = NULL;	
	}
	
	return pNode;
}

/*****************************************************
**FUNCTION	::	ChipGetFirst
**ACTION	::	Retrieve the first chip handle
**PARAMS IN	::	NONE
**PARAMS OUT	::	NONE
**RETURN	::	STCHIP_Handle_t if ok, NULL otherwise
*****************************************************/
static STCHIP_Handle_t ChipGetFirst(void)
{
	if((pFirstNode != NULL) && (pFirstNode->hChip != NULL))
		return pFirstNode->hChip;
	else
		return NULL;
}

/*****************************************************
**FUNCTION	::	ChipFindNode
**ACTION	::	Find that node that contains the chip 
**PARAMS IN	::	NONE
**PARAMS OUT	::	NONE
**RETURN	::	STCHIP_Handle_t if ok, NULL otherwise
*****************************************************/
static NODE *ChipFindNode(STCHIP_Handle_t hChip)
{
	NODE *pNode = pFirstNode;
	
	if(pNode != NULL)
	{
		while((pNode->hChip != hChip) && (pNode->pNextNode != NULL))
			pNode = pNode->pNextNode;
		
		if(pNode->hChip != hChip)
			pNode = NULL;
	}
	
	return pNode;
}

/*****************************************************
**FUNCTION	::	ChipGetNext
**ACTION	::	Retrieve the handle of the next chip
**PARAMS IN	::	hPrevChip==> handle of the previous chip
**PARAMS OUT	::	NONE
**RETURN	::	STCHIP_Handle_t if ok, NULL otherwise
*****************************************************/
static STCHIP_Handle_t ChipGetNext(STCHIP_Handle_t hPrevChip)
{
	NODE *pNode;
	
	pNode = ChipFindNode(hPrevChip);
	if((pNode != NULL) && (pNode->pNextNode != NULL))
		return pNode->pNextNode->hChip;
	else
		return NULL;
}

/*****************************************************
**FUNCTION	::	ChipGetHandleFromName
**ACTION	::	Retrieve the handle of chip with its name
**PARAMS IN	::	Name	==> name of the chip
**PARAMS OUT	::	NONE
**RETURN	::	STCHIP_Handle_t if ok, NULL otherwise
*****************************************************/
static STCHIP_Handle_t ChipGetHandleFromName(char *Name)
{
	STCHIP_Handle_t hChip;
	
	hChip = ChipGetFirst();
	while((hChip != NULL) && (strcmp(hChip->Name,Name) != 0))
	{
		hChip = ChipGetNext(hChip);
	}
	
	return hChip;
}

/*****************************************************
**FUNCTION	::	ChipOpen
**ACTION	::	Open a new chip
**PARAMS IN	::	Name	==> Name of the chip
**			I2cAddr	==> I2C address of the chip
**			NbRegs	==> number of register in the chip
**			NbFields==> number of field in the chip
**PARAMS OUT	::	NONE
**RETURN	::	Handle to the chip, NULL if an error occur
*****************************************************/
STCHIP_Handle_t ChipOpen(STCHIP_Info_t *hChipOpenParams)
{
	STCHIP_Handle_t hChip;
		
	/* Allocation of the chip structure	*/
	hChip = (STCHIP_Handle_t)kmalloc(sizeof(STCHIP_Info_t), GFP_KERNEL);

	if((hChip != NULL) && (hChipOpenParams != NULL))
	{
		/* Allocation of the register map*/
		hChip->pRegMap = (STCHIP_Register_t *)kmalloc(hChipOpenParams->NbRegs*sizeof(STCHIP_Register_t), GFP_KERNEL);

		if(hChip->pRegMap != NULL)
		{
			/* Allocation of the field map*/
			hChip->pFieldMap = (STCHIP_Field_t *)kmalloc(hChipOpenParams->NbFields*sizeof(STCHIP_Field_t), GFP_KERNEL);

			if(hChip->pFieldMap != NULL)
			{
				STCHIP_Handle_t hhChip;
				NODE *pNode = NULL;

				hhChip = ChipGetHandleFromName(hChipOpenParams->Name);
				
				if(hhChip==NULL)
					pNode = AppendNode(hChip);

				if ((hhChip==NULL) && (pNode==NULL))
				{
					kfree(hChip->pFieldMap);
					kfree(hChip->pRegMap);
					kfree(hChip);
					hChip = NULL;
				}
				else
				{
					hChip->I2cAddr = hChipOpenParams->I2cAddr;
					hChip->I2c_adap = hChipOpenParams->I2c_adap;
					strcpy(hChip->Name,hChipOpenParams->Name);
					hChip->NbRegs = hChipOpenParams->NbRegs;
					hChip->NbFields = hChipOpenParams->NbFields;
					hChip->ChipMode = hChipOpenParams->ChipMode;
					hChip->Repeater = hChipOpenParams->Repeater;
					hChip->RepeaterHost = hChipOpenParams->RepeaterHost;
					hChip->RepeaterFn = hChipOpenParams->RepeaterFn;
					hChip->WrStart  = hChipOpenParams->WrStart;
					hChip->WrSize   = hChipOpenParams->WrSize;
					hChip->RdStart  = hChipOpenParams->RdStart;
					hChip->RdSize   = hChipOpenParams->RdSize;
					hChip->ChipError = CHIPERR_NO_ERROR;
				}
			}
			else
			{
				kfree(hChip->pRegMap);
				kfree(hChip);
				hChip = NULL;
			}
		}
		else
		{
			kfree(hChip);
			hChip = NULL;	
		}
	}
	return hChip;
}

/*****************************************************
**FUNCTION	::	ChipAddReg
**ACTION	::	Add a new register to the register map
**PARAMS IN	::	hChip	==> Handle to the chip
**			Id	==> Id of the register
**			Name	==> Name of the register
**			Address	==> I2C address of the register
**			Default	==> Default value of the register
**PARAMS OUT	::	NONE
**RETURN	::	Error
*****************************************************/
STCHIP_Error_t ChipAddReg(STCHIP_Handle_t hChip, STCHIP_RegSize_t Size, u16 RegId, char * Name, u16 Address, u32 Default, STCHIP_Access_t Access, STCHIP_Pointed_t Pointed, u16 PointerRegAddr, u32 BaseAdress)
{
	STCHIP_Register_t *pReg;
	
	if(hChip != NULL)
	{	
		//if((RegId >= 0) && (RegId < hChip->NbRegs))
		if (RegId < hChip->NbRegs)
		{
			pReg = &hChip->pRegMap[RegId];
	
			pReg->Addr = Address;
			pReg->Size = Size;
			pReg->Default = Default;
			pReg->Value = Default;
			pReg->Access = Access;
			strcpy(pReg->Name, Name);
			pReg->Pointed = Pointed;
			pReg->PointerRegAddr = PointerRegAddr;
			pReg->BaseAdress = BaseAdress;
		}
		else
		{
			hChip->ChipError = CHIPERR_INVALID_REG_ID;
		}
	}
	else
		return CHIPERR_INVALID_HANDLE;

	return hChip->ChipError;
}

/*****************************************************
**FUNCTION	::	CreateMask
**ACTION	::	Create a mask according to its number of bits and position 
**PARAMS IN	::	field	==> Id of the field
**PARAMS OUT	::	NONE
**RETURN	::	mask of the field
*****************************************************/
static u32 CreateMask(char NbBits, char Pos)
{
	int i;
	u32 mask=0;
	
	/*Create mask*/
	for (i = 0; i < NbBits; i++)
	{
		mask <<= 1 ;
		mask +=  1 ;
	}
	mask = mask << Pos;
	
	return mask;
}

/*****************************************************
**FUNCTION	::	ChipAddField
**ACTION	::	Add a field to the field map 
**PARAMS IN	::	hChip	==> Handle to the chip
**			RegId	==> Id of the register which contains the field
**			FieldId	==> Id of the field
**			Name	==> Name of the field
**			Pos	==> Position (number of left shifts from LSB position) in the register 
**			NbBits	==> Size (in bits) of the field
**			Type	==> SIGNED or UNSIGNED
**PARAMS OUT	::	NONE
**RETURN	::	Error
*****************************************************/
STCHIP_Error_t ChipAddField(STCHIP_Handle_t hChip, u16 RegId, u32 FieldId, char * Name, char Pos, char NbBits, STCHIP_FieldType_t Type)
{
	STCHIP_Field_t *pField; 
	
	if(hChip != NULL)
	{
		if(RegId < hChip->NbRegs)
		{
			if(FieldId < (u32)(hChip->NbFields))
			{
				pField = &hChip->pFieldMap[FieldId];
	
				strcpy(pField->Name, Name);	
				pField->Reg = RegId;
				pField->Pos = Pos;
				pField->Bits = NbBits;
				pField->Type = Type;
				if(NbBits)
					pField->Mask = CreateMask(NbBits,Pos);
				else
					hChip->ChipError = CHIPERR_INVALID_FIELD_SIZE;
			}
			else
				hChip->ChipError = CHIPERR_INVALID_FIELD_ID;
		}
		else
		{
			hChip->ChipError = CHIPERR_INVALID_REG_ID;
		}
	}
	else 
		return CHIPERR_INVALID_HANDLE;
	
	return hChip->ChipError;
}

/*****************************************************
**FUNCTION	::	ChipSetOneRegister
**ACTION	::	Set the value of one register 
**PARAMS IN	::	hChip	==> Handle to the chip
**			reg_id	==> Id of the register
**			Data	==> Data to store in the register
**PARAMS OUT	::	NONE
**RETURN	::	Error
*****************************************************/
STCHIP_Error_t ChipSetOneRegister(STCHIP_Handle_t hChip,u16 RegId,u32 Data)   
{	
	if(hChip)
	{
		if(RegId < hChip->NbRegs)
		{
			if(hChip->pRegMap[RegId].Access != STCHIP_ACCESS_R)
			{
				hChip->pRegMap[RegId].Value = Data;
			
				if(hChip->ChipMode != STCHIP_MODE_NOSUBADR)
					ChipSetRegisters(hChip, RegId, 1);
				else
					ChipSetRegisters(hChip, hChip->WrStart, hChip->WrSize);
			}
		}
		else
		{
			hChip->ChipError = CHIPERR_INVALID_REG_ID;
		}
	}
	else
		return CHIPERR_INVALID_HANDLE;
	
	return hChip->ChipError;
}

/*****************************************************
**FUNCTION	::	ChipGetOneRegister
**ACTION	::	Get the value of one register 
**PARAMS IN	::	hChip	==> Handle to the chip
**				reg_id	==> Id of the register
**PARAMS OUT::	NONE
**RETURN	::	Register's value
*****************************************************/
int ChipGetOneRegister(STCHIP_Handle_t hChip, u16 RegId)
{
	u32 data = 0xFFFFFFFF;
	
	if(hChip)
	{	
		if((hChip->ChipMode != STCHIP_MODE_NOSUBADR) && (hChip->ChipMode != STCHIP_MODE_NO_R_SUBADR))
		{
			if(ChipGetRegisters(hChip,RegId,1) == CHIPERR_NO_ERROR)
				data = hChip->pRegMap[RegId].Value;
		}
		else
		{
			if(ChipGetRegisters(hChip, hChip->RdStart, hChip->RdSize) == CHIPERR_NO_ERROR)
				data = hChip->pRegMap[RegId].Value;
		}
	}
	
	return data;
}

/*****************************************************
**FUNCTION	::	ChipSetRegisters
**ACTION	::	Set values of consecutive's registers (values are taken in RegMap)
**PARAMS IN	::	hChip==> Handle to the chip
**			FirstReg==> Id of the first register
**			NbRegs	==> Number of register to write		
**PARAMS OUT	::	NONE
**RETURN	::	Error
*****************************************************/
STCHIP_Error_t ChipSetRegisters(STCHIP_Handle_t hChip, int FirstReg, int NbRegs)
{
    unsigned char data[100],nbdata = 0;
    int i,j;
	//printk(KERN_ERR "ChipSetRegisters--->\n");
    if(hChip)
    {
	if(!hChip->ChipError)
	{
	    if(NbRegs < 20)
	    {
		if((FirstReg >= 0) && ((FirstReg + NbRegs - 1) < hChip->NbRegs))
		{
		    if(hChip->pRegMap[FirstReg].Pointed == STCHIP_POINTED)
		    {
			if((hChip->pRegMap[FirstReg].PointerRegAddr != LastPointer) || (hChip->pRegMap[FirstReg].BaseAdress != LastBaseAdress))
			{
			    /* Write pointer register (4x8bits read access) */
			    nbdata=0;
			    data[nbdata++] = MSB(hChip->pRegMap[FirstReg].PointerRegAddr);
			    data[nbdata++] = LSB(hChip->pRegMap[FirstReg].PointerRegAddr);

			    for(i=0;i<4;i++) 
				data[nbdata++] = (unsigned char)((hChip->pRegMap[FirstReg].BaseAdress>>(8*i))&0xFF);
			    /* write base adress value */		
			    if((I2cWrite(hChip,data,nbdata)) < 0)
				hChip->ChipError = CHIPERR_I2C_NO_ACK;

			    LastPointer = hChip->pRegMap[FirstReg].PointerRegAddr;
			    LastBaseAdress = hChip->pRegMap[FirstReg].BaseAdress;
			}			
			/* Write pointed register (4x8bits read access) */ 
			nbdata = 0;
			data[nbdata++] = MSB(hChip->pRegMap[FirstReg].Addr);	
			data[nbdata++] = LSB(hChip->pRegMap[FirstReg].Addr);	
						
			for(j=0; j<NbRegs; j++)
			    for(i=0;i<4;i++) 
				data[nbdata++] = (unsigned char)((hChip->pRegMap[FirstReg+j].Value>>(8*i))&0xFF);
			/* write base adress value */
			if(I2cWrite(hChip, data, nbdata) < 0)
			    hChip->ChipError = CHIPERR_I2C_NO_ACK;
		    }
		    else
		    {	
			nbdata = 0;
			switch(hChip->ChipMode)
			{
			    case STCHIP_MODE_SUBADR_16:
				data[nbdata++] = MSB(hChip->pRegMap[FirstReg].Addr);
			    case STCHIP_MODE_SUBADR_8:
			    case STCHIP_MODE_NO_R_SUBADR:
				data[nbdata++] = LSB(hChip->pRegMap[FirstReg].Addr);
			    case STCHIP_MODE_NOSUBADR:
				for(i=0;i<NbRegs;i++) /* fill data buffer */
				    data[nbdata++] = (unsigned char)((hChip->pRegMap[FirstReg+i].Value)&0xFF);
			    break;
			}

			if(hChip->Repeater)
			    hChip->RepeaterFn(hChip->RepeaterHost, TRUE);

			//msleep(1);

			if(I2cWrite(hChip,data,nbdata) < 0)
			{
			    hChip->ChipError = CHIPERR_I2C_NO_ACK;
			}

			if(hChip->Repeater)
			    hChip->RepeaterFn(hChip->RepeaterHost,FALSE);
		    }
		}
		else
		{
		    hChip->ChipError = CHIPERR_INVALID_REG_ID;
		}
	    }
	    else
	    hChip->ChipError = CHIPERR_I2C_BURST;
	}
    }
    else
	return CHIPERR_INVALID_HANDLE;
	//printk(KERN_ERR "ChipSetRegisters<---\n");
    return hChip->ChipError;
}

/*****************************************************
**FUNCTION	::	ChipGetRegisters
**ACTION	::	Get values of consecutive's registers (values are writen in RegMap)
**PARAMS IN	::	hChip	==> Handle to the chip
**			FirstReg==> Id of the first register
**			NbRegs	==> Number of register to read		
**PARAMS OUT	::	NONE
**RETURN	::	Error
*****************************************************/
STCHIP_Error_t ChipGetRegisters(STCHIP_Handle_t hChip, int FirstReg, int NbRegs)
{
    u8 data[100],dataread[100],dummy[4],nbdata =0;
    u16 address;
    u8 dum;
    int i,j=0;
	
    if(hChip)
    {
	if(!hChip->ChipError)
	{
	    if(NbRegs < 20)
	    {
		if((FirstReg >= 0) && ((FirstReg + NbRegs - 1) < hChip->NbRegs))
		{
		    if(hChip->pRegMap[FirstReg].Pointed == STCHIP_POINTED)
		    {
			if((hChip->pRegMap[FirstReg].PointerRegAddr != LastPointer) || (hChip->pRegMap[FirstReg].BaseAdress != LastBaseAdress))
			{
			    /* Write pointer register (4x8bits write access) */
			    nbdata = 0;
			    data[nbdata++] = MSB(hChip->pRegMap[FirstReg].PointerRegAddr);
			    data[nbdata++] = LSB(hChip->pRegMap[FirstReg].PointerRegAddr);

			    for(i=0; i<4; i++)
				data[nbdata++] = (unsigned char)((hChip->pRegMap[FirstReg].BaseAdress>>(8*i))&0xFF);

			    if(I2cWrite(hChip,data,nbdata) < 0)
			    	hChip->ChipError = CHIPERR_I2C_NO_ACK;

			    LastPointer = hChip->pRegMap[FirstReg].PointerRegAddr;
			    LastBaseAdress = hChip->pRegMap[FirstReg].BaseAdress;
			}	
			nbdata = 0;
			if((LSB(hChip->pRegMap[FirstReg].Addr) & 0x08) == 0)
			    dum = 0x20;
			else
			    dum = 0;
			dummy[nbdata++] = MSB(hChip->pRegMap[FirstReg].Addr);
			dummy[nbdata++] = dum;
			/* write register adress+20 value */
			if(I2cWrite(hChip, dummy, nbdata) < 0)
			    hChip->ChipError = CHIPERR_I2C_NO_ACK;

			if(I2cRead(hChip,dummy,4) < 0)
			    hChip->ChipError = CHIPERR_I2C_NO_ACK;
	
			/* Read pointed register (4x8bits read access) */
			nbdata = 0;
			address = hChip->pRegMap[FirstReg].Addr;
			data[nbdata++] = MSB(address);
			data[nbdata++] = LSB(address);
					
			if(I2cWrite(hChip,data,nbdata) < 0)
			    hChip->ChipError = CHIPERR_I2C_NO_ACK;
	
			if(I2cRead(hChip,dataread,(4*NbRegs)) < 0)
			    hChip->ChipError = CHIPERR_I2C_NO_ACK;
			memcpy(data, dataread, (4*NbRegs));
		    }
		    else
		    {
			nbdata = 0;
			switch(hChip->ChipMode)
			{	
			    case STCHIP_MODE_SUBADR_16:
				data[nbdata++] = MSB(hChip->pRegMap[FirstReg].Addr);
			    case STCHIP_MODE_SUBADR_8:
				data[nbdata++]=LSB(hChip->pRegMap[FirstReg].Addr);

			    if(hChip->Repeater)
				hChip->RepeaterFn(hChip->RepeaterHost, TRUE);	
			    /* Write sub address */
			    if(I2cWrite(hChip,data,nbdata) < 0)
				hChip->ChipError = CHIPERR_I2C_NO_ACK;

			    if(hChip->Repeater)
				hChip->RepeaterFn(hChip->RepeaterHost, FALSE);

			    case STCHIP_MODE_NOSUBADR:
			    case STCHIP_MODE_NO_R_SUBADR:
				if(hChip->Repeater)
				    hChip->RepeaterFn(hChip->RepeaterHost, TRUE);
	
				nbdata = 0;
				if(hChip->pRegMap[FirstReg].Pointed == STCHIP_POINTED)
				    for(i=0; i<NbRegs; i++)
					nbdata += 4;
				else
					nbdata = (unsigned char)NbRegs;

				if(I2cRead(hChip,data,nbdata) < 0)
				    hChip->ChipError = CHIPERR_I2C_NO_ACK;

				if(hChip->Repeater)
				    hChip->RepeaterFn(hChip->RepeaterHost, FALSE);

				/*only for inbuf and packet delin registers*/
				if((MSB(hChip->pRegMap[FirstReg].Addr) == 0xf6) || (MSB(hChip->pRegMap[FirstReg].Addr) == 0xf2))
				{
				    dummy[0] = MSB(hChip->pRegMap[FirstReg].Addr);
				    dummy[1] = 0xff;

				    if(I2cWrite(hChip,dummy,2) < 0)
					hChip->ChipError = CHIPERR_I2C_NO_ACK;

				    if(I2cRead(hChip,dummy,1) < 0)
					hChip->ChipError = CHIPERR_I2C_NO_ACK;
				}
			    break;
			}
		    }
		    /*Update RegMap structure	*/
		    for(i=0; i<NbRegs; i++)
		    if(!hChip->ChipError)
		    {
			hChip->pRegMap[FirstReg+i].Value = data[j++];

			if(hChip->pRegMap[FirstReg+i].Size >= STCHIP_REG_16)
			    hChip->pRegMap[FirstReg+i].Value += data[j++]<<8;
	
			if(hChip->pRegMap[FirstReg+i].Size >= STCHIP_REG_24)
			    hChip->pRegMap[FirstReg+i].Value += data[j++]<<16;

			if(hChip->pRegMap[FirstReg+i].Size >= STCHIP_REG_32)
			    hChip->pRegMap[FirstReg+i].Value += data[j++]<<24;
		    }
		    else
		    hChip->pRegMap[FirstReg+i].Value = 0xFFFFFFFF;
		}
		else
		{
		    hChip->ChipError = CHIPERR_INVALID_REG_ID;
		}
	    }
	    else 
		hChip->ChipError = CHIPERR_I2C_BURST;
	}
    }
    else
	return CHIPERR_INVALID_HANDLE;

    return hChip->ChipError;
}

/*****************************************************
**FUNCTION	::	ChipSetFieldImage
**ACTION	::	Set value of a field in RegMap
**PARAMS IN	::	hChip	==> Handle to the chip
**			FieldId	==> Id of the field
**			Value	==> Value of the field
**PARAMS OUT	::	NONE
**RETURN	::	Error
*****************************************************/
STCHIP_Error_t ChipSetFieldImage(STCHIP_Handle_t hChip, u32 FieldId, int Value)
{
	STCHIP_Field_t *pfield;
	
	if(hChip != NULL)
	{
		if(!hChip->ChipError)
		{
			if(FieldId < (u32)(hChip->NbFields))
			{
				pfield = &hChip->pFieldMap[FieldId];

				if(pfield->Type == CHIP_SIGNED)
					/*compute signed fieldval*/
					Value = (Value > 0 ) ? Value : Value + (1<<pfield->Bits);

				/*Shift and mask value*/
				Value = pfield->Mask & (Value << pfield->Pos);
				/*Concat register value and fieldval*/
				hChip->pRegMap[pfield->Reg].Value = (hChip->pRegMap[pfield->Reg].Value & (~pfield->Mask)) + Value;
			}
			else
				hChip->ChipError = CHIPERR_INVALID_FIELD_ID;
		}
	}
	else
		return CHIPERR_INVALID_HANDLE;

	return hChip->ChipError;
}

/*****************************************************
**FUNCTION	::	ChipSetField
**ACTION	::	Set value of a field in the chip
**PARAMS IN	::	hChip	==> Handle to the chip
**			FieldId	==> Id of the field
**			Value	==> Value to write
**PARAMS OUT	::	NONE
**RETURN	::	Error
*****************************************************/
STCHIP_Error_t ChipSetField(STCHIP_Handle_t hChip, u32 FieldId, int Value)
{
	STCHIP_Field_t *pfield;
	
	if(hChip)
	{
		if(!hChip->ChipError)
		{
			if(FieldId < (u32)(hChip->NbFields))
			{
				/*Just for code simplification*/
				pfield = &hChip->pFieldMap[FieldId];
				/*Read the register*/
				ChipGetOneRegister(hChip,pfield->Reg);
				/*Compute new RegMap value */
				ChipSetFieldImage(hChip,FieldId,Value);
				/*Write the register */
				ChipSetOneRegister(hChip, pfield->Reg, hChip->pRegMap[pfield->Reg].Value);
			}
			else
				hChip->ChipError = CHIPERR_INVALID_FIELD_ID;
		}
	}
	else
		return CHIPERR_INVALID_HANDLE;
		
	return hChip->ChipError;
}

/*****************************************************
**FUNCTION	::	ChipGetFieldImage
**ACTION	::	get the value of a field from RegMap
**PARAMS IN	::	hChip	==> Handle of the chip
**			FieldId	==> Id of the field
**PARAMS OUT	::	NONE
**RETURN	::	field's value
*****************************************************/
int ChipGetFieldImage(STCHIP_Handle_t hChip, u32 FieldId)
{
	int value = 0xFFFFFFFF;
	STCHIP_Field_t *pfield;
	
	if(hChip)
	{
		if(FieldId < (u32)(hChip->NbFields))
		{
			pfield = &hChip->pFieldMap[FieldId];
			if(!hChip->ChipError)
				value = hChip->pRegMap[pfield->Reg].Value;
				
			value = (value & pfield->Mask) >> pfield->Pos; /*Extract field*/

			if((pfield->Type == CHIP_SIGNED) && (value & (1 << (pfield->Bits-1))))
				value = value - (1<<pfield->Bits);/*Compute signed value*/
		}
		else
			hChip->ChipError = CHIPERR_INVALID_FIELD_ID;
	}
	
	return value;
}

/*****************************************************
**FUNCTION	::	ChipGetField
**ACTION	::	get the value of a field from the chip
**PARAMS IN	::	hChip	==> Handle of the chip
**			FieldId	==> Id of the field
**PARAMS OUT	::	NONE
**RETURN	::	field's value
*****************************************************/
int ChipGetField(STCHIP_Handle_t hChip,u32 FieldId)
{
	int value = 0xFFFFFFFF;
	
	if(hChip)
	{
		if(FieldId < (u32)(hChip->NbFields))
		{
			ChipGetOneRegister(hChip, hChip->pFieldMap[FieldId].Reg);
			value = ChipGetFieldImage(hChip, FieldId);
		}
		else
			hChip->ChipError = CHIPERR_INVALID_FIELD_ID;
	}
	
	return value;
}

/*****************************************************
**FUNCTION	::	ChipApplyDefaultValues
**ACTION	::	Write default values in all the registers
**PARAMS IN	::	hChip	==> Handle of the chip	
**PARAMS OUT	::	NONE
**RETURN	::	Error
*****************************************************/
STCHIP_Error_t ChipApplyDefaultValues(STCHIP_Handle_t hChip)
{
	int reg = 0;

	if(hChip != NULL) 
	{
		if(hChip->ChipMode != STCHIP_MODE_NOSUBADR)
		{
			while((!hChip->ChipError) && (reg < hChip->NbRegs))
			{
				ChipSetOneRegister(hChip,(u16)reg,hChip->pRegMap[reg].Default);
				reg++;
			}
		}
		else
		{	printk(KERN_ERR "hChip->ChipMode == STCHIP_MODE_NOSUBADR\n");
			ChipSetOneRegister(hChip,(u16)reg,hChip->pRegMap[reg].Default);
		}
	}
	else
		return CHIPERR_INVALID_HANDLE;

	return hChip->ChipError;
}

