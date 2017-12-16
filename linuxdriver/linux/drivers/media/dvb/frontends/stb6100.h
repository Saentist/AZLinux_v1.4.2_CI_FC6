
  /*
     Driver for Philips tda8262/tda8263 DVBS Silicon tuners

     (c) 2006 Andrew de Quincey

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  */

#ifndef __DVB_STB6100_H__
#define __DVB_STB6100_H__

#include <linux/i2c.h>
#include "dvb_frontend.h"

	#define NBREGS	12

	/* RFMagic STB6100 tuner definition */ 	

	/*	LD	*/
	#define LD		0
	#define LD		0

	/*	VCO	*/
	#define VCO		1
	#define OSCH		1
	#define OCK		2
	#define ODIV		3
	#define OSM		4

	/*	NI	*/
	#define R_NI		2
	#define F_NI		5

	/*	NF_LSB	*/
	#define R_NF_LSB		3
	#define F_NF_LSB		6

	/*	K	*/
	#define R_K		4
	#define F_K		7
	#define F_PSD2		8
	#define F_NF_MSB		9

	/*	G	*/
	#define R_G		5
	#define F_GCT		10
	#define F_G		11

	/*	F	*/
	#define R_F		6
	#define F_F		12

	/*	DLB	*/
	#define R_DLB		7
	#define F_DLB		13

	/*	TEST1	*/
	#define R_TEST1		8
	#define F_TEST1		14

	/*	TEST2	*/
	#define R_TEST2		9
	#define F_FCCK		15

	/*	LPEN	*/
	#define R0_LPEN		10
	#define F_BEN		16
	#define F_OSCP		17
	#define F_SYNP		18
	#define F_LPEN		19

	/*	TEST3	*/
	#define R_TEST3		11
	#define F_TEST3		20

	typedef enum
	{
		TUNER_IQ_NORMAL = 1,
		TUNER_IQ_INVERT = -1
	}
	TUNER_IQ_t;
	
extern struct dvb_frontend *stb6100_attach(struct dvb_frontend *fe, int addr, struct i2c_adapter *i2c);

#endif

