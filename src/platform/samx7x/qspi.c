/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#include <sam.h>

#include "platform/samx7x/pio.h"
#include "platform/samx7x/pmc.h"
#include "platform/samx7x/qspi.h"

#include "util/types.h"

void qspi_init_interface(enum qspi_mode mode, u32 frequency)
{
	struct pmc_clock_tree_t clock_tree = pmc_get_clock_tree();

	/* Enable peripheral clock */
	pmc_generic_clock_config(QSPI_CLOCK_ID, 1, PMC_PCR_GCLKCSS_MAIN_CLK, (clock_tree.mainck_freq / frequency));

	/* Reset control SPI peripheral */
	QSPI->QSPI_CR |= QSPI_CR_SWRST;
	/* Configure control SPI peripheral */
	QSPI->QSPI_MR = QSPI_MR_NBBITS_8_BIT | QSPI_MR_SMM_SPI;

	/* Configure control SPI peripheral */
	switch (mode) {
	SPI_MODE0:
		/* CPOL = 0, CPHA = 0*/
		QSPI->QSPI_SCR = 0;
		break;

	SPI_MODE1:
		/* CPOL = 0, CPHA = 1*/
		QSPI->QSPI_SCR = QSPI_SCR_CPHA;
		break;

	SPI_MODE2:
		/* CPOL = 1, CPHA = 0 */
		QSPI->QSPI_SCR = QSPI_SCR_CPOL;
		break;

	SPI_MODE3:
		/* CPOL = 1, CPHA = 1*/
		QSPI->QSPI_SCR = QSPI_SCR_CPOL | QSPI_SCR_CPHA;
		break;
	}
	/* Enable QSPI */
	QSPI->QSPI_CR = QSPI_CR_QSPIEN_Msk;
}

struct qspi_device_t qspi_init_device(struct pio_pin_t cs_pio, u8 cs_inverted)
{
	struct qspi_device_t device = {
		.cs_pio = cs_pio,
		.cs_inverted = !!cs_inverted,
	};

	return device;
}

void qspi_select(struct qspi_device_t device, u8 state)
{
	/* state 1 = selected, active low unless cs_inverted is set */
	pio_set(device.cs_pio, (!state) ^ device.cs_inverted);
}

u8 qspi_transfer_byte(struct qspi_device_t device, const u8 data)
{
	/* Transmission Registers Empty */
	while (!(QSPI->QSPI_SR & QSPI_SR_TXEMPTY_Msk)) continue;

	QSPI->QSPI_TDR = data;

	/* Receive Data Register Ful */
	while (!(QSPI->QSPI_SR & QSPI_SR_RDRF_Msk)) continue;

	return (u8) (QSPI->QSPI_RDR & 0xFF);
}

void qspi_transfer(struct qspi_device_t device, const u8 *src, u32 size, u8 *dst)
{
	if (src) {
		while (size--) {
			if (dst)
				*(dst++) = qspi_transfer_byte(device, *(src++));
			else
				qspi_transfer_byte(device, *(src++));
		}
	} else if (dst) {
		while (size--) *(dst++) = qspi_transfer_byte(device, 0x00);
	}
}
