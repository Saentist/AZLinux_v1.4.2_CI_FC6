/*
	Mantis PCI bridge driver

	Copyright (C) 2005, 2006 Manu Abraham (abraham.manu@gmail.com)

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

#include <asm/page.h>
#include <linux/vmalloc.h>
#include "mantis_common.h"

#define RISC_WRITE		(0x01 << 28)
#define RISC_JUMP		(0x07 << 28)
#define RISC_SYNC		(0x08 << 28)

#define RISC_WR_SOL		(1 << 27)
#define RISC_WR_EOL		(1 << 26)
#define RISC_IRQ		(1 << 24)

#define RISC_STATUS(status)	((((~status) & 0x0f) << 20) | ((status & 0x0f) << 16))

#define RISC_FLUSH()		mantis->risc_pos = 0
#define RISC_INSTR(opcode)	mantis->risc_cpu[mantis->risc_pos++] = cpu_to_le32(opcode)


static void mantis_free(struct mantis_pci *mantis)
{
	if (mantis->buf_cpu) {
		dprintk(verbose, MANTIS_ERROR, 1,
			"DMA=%lx", (unsigned long) mantis->buf_dma);

		pci_free_consistent(mantis->pdev, mantis->buf_size,
				    mantis->buf_cpu, mantis->buf_dma);

		mantis->buf_cpu = NULL;
	}
	if (mantis->risc_cpu) {
		dprintk(verbose, MANTIS_ERROR, 1,
			"RISC=%lx", (unsigned long) mantis->risc_dma);

		pci_free_consistent(mantis->pdev, mantis->risc_size,
				    mantis->risc_cpu, mantis->risc_dma);

		mantis->risc_cpu = NULL;
	}
}

static int mantis_alloc_buffers(struct mantis_pci *mantis)
{
	if (!mantis->buf_cpu) {
		mantis->buf_size = 64 * 1024;
		mantis->buf_cpu = pci_alloc_consistent(mantis->pdev,
						       mantis->buf_size,
						       &mantis->buf_dma);
		if (!mantis->buf_cpu) {
			dprintk(verbose, MANTIS_ERROR, 1,
				"DMA buffer allocation failed");

			goto err;
		}
		dprintk(verbose, MANTIS_ERROR, 1,
			"DMA=0x%lx cpu=0x%p size=%d",
			(unsigned long) mantis->buf_dma,
			mantis->buf_cpu, mantis->buf_size);
	}
	if (!mantis->risc_cpu) {
		mantis->risc_size = PAGE_SIZE;
		mantis->risc_cpu = pci_alloc_consistent(mantis->pdev,
							mantis->risc_size,
							&mantis->risc_dma);

		if (!mantis->risc_cpu) {
			dprintk(verbose, MANTIS_ERROR, 1,
				"RISC program allocation failed");

			mantis_free(mantis);

			goto err;
		}
		dprintk(verbose, MANTIS_ERROR, 1,
			"RISC=0x%lx cpu=0x%p size=%d",
			(unsigned long) mantis->risc_dma,
			mantis->risc_cpu, mantis->risc_size);
	}

	return 0;
err:
	dprintk(verbose, MANTIS_ERROR, 1, "Out of memory (?) .....");
	return -ENOMEM;
}

static int mantis_calc_lines(struct mantis_pci *mantis)
{
	// (64 * 1024) / 16 = 4096
	mantis->block_bytes = mantis->buf_size >> 4;
	mantis->block_count = 1 << 4;
	mantis->line_bytes = mantis->block_bytes;
	mantis->line_count = mantis->block_count;

	// bit 12
	while (mantis->line_bytes > 4095) {
		mantis->line_bytes >>= 1;
		mantis->line_count <<= 1;
	}
	dprintk(verbose, MANTIS_ERROR, 1,
		"Mantis RISC block bytes=[%d], line bytes=[%d], line count=[%d]",
		mantis->block_bytes, mantis->line_bytes, mantis->line_count);

	if (mantis->line_count > 255) {
		dprintk(verbose, MANTIS_ERROR, 1, "Buffer size error");
		return -EINVAL;
	}

	return 0;
}

static void mantis_risc_program(struct mantis_pci *mantis)
{
	u32 buf_pos = 0;
	u32 line;

	dprintk(verbose, MANTIS_DEBUG, 1, "Mantis create RISC program");
	RISC_FLUSH();

	dprintk(verbose, MANTIS_DEBUG, 1, "risc len lines %u, bytes per line %u",
		mantis->line_count, mantis->line_bytes);

	for (line = 0; line < mantis->line_count; line++) {
		//dprintk(verbose, MANTIS_ERROR, 1, "RISC PROG line=[%d]", line);
		if (!((buf_pos/8) % mantis->block_bytes)) {
			RISC_INSTR(RISC_WRITE	|
				   RISC_IRQ	|
				   RISC_STATUS(((buf_pos / mantis->block_bytes) +
				   (mantis->block_count - 1)) %
				    mantis->block_count) |
				    mantis->line_bytes);
		} else {
			RISC_INSTR(RISC_WRITE	|
				   mantis->line_bytes);
		}
		RISC_INSTR(mantis->buf_dma + buf_pos);
		buf_pos += mantis->line_bytes;
	}
	
	RISC_INSTR(RISC_JUMP);
	RISC_INSTR(mantis->risc_dma);
}

void mantis_dma_start(struct mantis_pci *mantis)
{
	dprintk(verbose, MANTIS_ERROR, 1, "Mantis Start DMA engine");
	mantis_risc_program(mantis);
	// RISC program start address
	mmwrite(cpu_to_le32(mantis->risc_dma), MANTIS_RISC_START);

	// initialize capture, FIFO an RISC
	mmwrite(0, MANTIS_DMA_CTL);
	mantis->last_block = mantis->finished_block = 0;

	// RISC interrupts
	mmwrite(mmread(MANTIS_INT_MASK) | MANTIS_INT_RISCI, MANTIS_INT_MASK);

	// RISC engine start
	mmwrite(MANTIS_FIFO_EN | MANTIS_DCAP_EN
			       | MANTIS_RISC_EN, MANTIS_DMA_CTL);

}

void mantis_dma_stop(struct mantis_pci *mantis)
{
	u32 stat = 0, mask = 0;

	stat = mmread(MANTIS_INT_STAT);
	mask = mmread(MANTIS_INT_MASK);
	dprintk(verbose, MANTIS_DEBUG, 1, "Mantis Stop DMA engine");

	// Disable capture, FIFO, RISC Engine
	mmwrite((mmread(MANTIS_DMA_CTL) & ~(MANTIS_FIFO_EN |
					    MANTIS_DCAP_EN |
					    MANTIS_RISC_EN)), MANTIS_DMA_CTL);

	// Clear all current interrupts
	mmwrite(mmread(MANTIS_INT_STAT), MANTIS_INT_STAT);

	// Disable RISC interrupts
	mmwrite(mmread(MANTIS_INT_MASK) & ~(MANTIS_INT_RISCI |
					    MANTIS_INT_RISCEN), MANTIS_INT_MASK);
}

int mantis_dma_init(struct mantis_pci *mantis)
{
	int err = 0;

	dprintk(verbose, MANTIS_ERROR, 1, "Mantis DMA init");
	if (mantis_alloc_buffers(mantis) < 0) {
		dprintk(verbose, MANTIS_ERROR, 1, "Error allocating DMA buffer");

		// Stop RISC Engine
		mmwrite(0, MANTIS_DMA_CTL);

		goto err;
	}
	if ((err = mantis_calc_lines(mantis)) < 0) {
		dprintk(verbose, MANTIS_ERROR, 1, "Mantis calc lines failed");

		goto err;
	}

	return 0;
err:
	return err;
}

void mantis_dma_xfer(unsigned long data)
{	
	struct mantis_pci *mantis = (struct mantis_pci *) data;

	while (mantis->last_block != mantis->finished_block) {
		dprintk(verbose, MANTIS_DEBUG, 1, "last block=[%d] finished block=[%d]",
			mantis->last_block, mantis->finished_block);

		(mantis->ts_size ? dvb_dmx_swfilter_204: dvb_dmx_swfilter)
		(&mantis->demux, &mantis->buf_cpu[mantis->last_block * mantis->block_bytes], mantis->block_bytes);
		mantis->last_block = (mantis->last_block + 1) % mantis->block_count;
	}
}

int mantis_dma_exit(struct mantis_pci *mantis)
{
	if (mantis->buf_cpu) {
		dprintk(verbose, MANTIS_ERROR, 1,
			"DMA=0x%lx cpu=0x%p size=%d",
			(unsigned long) mantis->buf_dma,
			 mantis->buf_cpu,
			 mantis->buf_size);

		pci_free_consistent(mantis->pdev, mantis->buf_size,
				    mantis->buf_cpu, mantis->buf_dma);

		mantis->buf_cpu = NULL;
	}
	if (mantis->risc_cpu) {
		dprintk(verbose, MANTIS_ERROR, 1,
			"RISC=0x%lx cpu=0x%p size=%d",
			(unsigned long) mantis->risc_dma,
			mantis->risc_cpu,
			mantis->risc_size);

		pci_free_consistent(mantis->pdev, mantis->risc_size,
				    mantis->risc_cpu, mantis->risc_dma);

		mantis->risc_cpu = NULL;
	}

	return 0;
}
