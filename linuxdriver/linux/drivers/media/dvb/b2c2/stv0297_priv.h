/*
 * Driver for the DVB-C STV0297 demodulator - chip stuff
 *
 *  Copyright (C) 2005 Patrick Boettcher <patrick.boettcher@desy.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc., 675
 *  Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef __STV0297_PRIV_INCLUDED__
#define __STV0297_PRIV_INCLUDED__

enum stv0297_registers {
	EQU_0			= 0x00,
	EQU_1,
	EQU_2,
	EQU_3,
	EQU_4,
	EQU_5,
	EQU_6,
	EQU_7,
	EQU_8,
	INITDEM_0		= 0x20,
	INITDEM_1,
	INITDEM_2,
	INITDEM_3,
	INITDEM_4,
	INITDEM_5,
	DELAGC_0		= 0x30,
	DELAGC_1,
	DELAGC_2,
	DELAGC_3,
	DELAGC_4,
	DELAGC_5,
	DELAGC_6,
	DELAGC_7,
	DELAGC_8,
	DELAGC_BLK_LEN  = 1+DELAGC_8-DELAGC_0,
	WBAGC_0			= 0x40,
	WBAGC_1,
	WBAGC_2,
	WBAGC_3,
	WBAGC_4,
	WBAGC_5,
	WBAGC_6,
	WBAGC_7,
	WBAGC_8,
	WBAGC_9,
	WBAGC_10,
	WBAGC_11,
	WBAGC_BLK_LEN   = 1+WBAGC_11-WBAGC_0,
	ST_LOOP2		= 0x52,
	ST_LOOP3,
	ST_LOOP4,  // Not used.
	ST_LOOP5,
	ST_LOOP6,
	ST_LOOP7,
	ST_LOOP8,
	ST_LOOP9,
	ST_LOOP10,
	ST_LOOP11,
	CRL_0			= 0x60,
	CRL_1,
	CRL_2,
	CRL_3,
	CRL_4,
	CRL_5,
	CRL_6,
	CRL_7,
	CRL_8,
	CRL_9,
	CRL_10,
	CRL_11,
	PMFAGC_0		= 0x70,
	PMFAGC_1,
	PMFAGC_2,
	PMFAGC_3,
	PMFAGC_4,
	CTRL_0			= 0x80,
	CTRL_1,
	CTRL_2,
	CTRL_3,
	CTRL_4,
	CTRL_5,
	CTRL_6,
	CTRL_7,
	CTRL_8,
	CTRL_9,
	DEINT_SYNC_0	= 0x90,
	DEINT_SYNC_1,
	BERT_0			= 0xA0,
	BERT_1,
	BERT_2,
	DEINT_0			= 0xB0,
	DEINT_1,
	DEINT_2,
	DEINT_3,
	OUTFORMAT_0		= 0xC0,
	OUTFORMAT_1,
	OUTFORMAT_2,
	RS_DESC_0		= 0xD0,
	RS_DESC_1,
	RS_DESC_2,
	RS_DESC_3,
	RS_DESC_4,
	RS_DESC_5,
	RS_DESC_14		=0xDE,
	RS_DESC_15
};

enum {
	WAGC_EN			= 0x10,
	STV0297_QAM16	= 0x00,
	STV0297_QAM32	= 0x10,
	STV0297_QAM64	= 0x40,
	STV0297_QAM128	= 0x20,
	STV0297_QAM256	= 0x30,
	STV0297_CLEARQAM = ~0x70
};

#define AGC2MAX			DELAGC_0
#define AGC2MIN			DELAGC_1
#define AGC1MAX			DELAGC_2
#define AGC1MIN			DELAGC_3
#define AGC2_THRESH		DELAGC_5

#define NO_AGC_FREEZE	0x80
#define SOFT_AGC_FREEZE	0x28

#define SPECTRAL_INVERSION_TOGGLE 0x08

#define MINIMUM_SYMBOL_RATE_IN_BAUD       1000000UL
#define MAXIMUM_SYMBOL_RATE_IN_BAUD       7000000UL
#define MAXIMUM_NTSC_SYMBOL_RATE          5300000UL

#define MINIMUM_FREQUENCY_IN_KHz         48000000UL
#define MAXIMUM_FREQUENCY_IN_KHz        860000000UL

/* Set to an invalid, non-zero figure. */
#define INITIAL_SYMBOL_RATE MAXIMUM_SYMBOL_RATE_IN_BAUD+1

#define TOGGLE_MASK 0x03

#endif
