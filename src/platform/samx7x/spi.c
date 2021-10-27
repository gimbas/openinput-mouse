/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#include <sam.h>

#include "platform/samx7x/pio.h"
#include "platform/samx7x/pmc.h"
#include "platform/samx7x/spi.h"

#include "util/types.h"

void spi_init_interface(enum spi_interface_no interface_no, enum spi_mode mode, u32 frequency)
{
	struct pmc_clock_tree_t clock_tree = pmc_get_clock_tree();
	u8 per_id;
	Spi *interface;

	switch (interface_no) {
		case SPI_INTERFACE_0:
			per_id = SPI0_CLOCK_ID;
			interface = SPI0;
			break;

#if defined(SPI1)
		case SPI_INTERFACE_1:
			per_id = SPI1_CLOCK_ID;
			interface = SPI1;
			break;
#endif
	}

	/* Enable peripheral clock */
	pmc_generic_clock_config(per_id, 1, PMC_PCR_GCLKCSS_MAIN_CLK, (clock_tree.mainck_freq / frequency));

	/* Reset control SPI peripheral */
	interface->SPI_CR |= SPI_CR_SWRST;
	/* Configure control SPI peripheral */
	interface->SPI_MR = SPI_MR_MSTR_MASTER;

	u32 mode_config = 0;

	switch (mode) {
	SPI_MODE0:
		/* CPOL = 0, CPHA = 0*/
		mode_config = SPI_CSR_CPOL(0) | SPI_CSR_NCPHA(0);
		break;

	SPI_MODE1:
		/* CPOL = 0, CPHA = 1*/
		mode_config = SPI_CSR_CPOL(0) | SPI_CSR_NCPHA(1);
		break;

	SPI_MODE2:
		/* CPOL = 1, CPHA = 0 */
		mode_config = SPI_CSR_CPOL(1) | SPI_CSR_NCPHA(0);
		break;

	SPI_MODE3:
		/* CPOL = 1, CPHA = 1*/
		mode_config = SPI_CSR_CPOL(1) | SPI_CSR_NCPHA(1);
		break;
	}
	/* Configure control SPI peripheral */
	interface->SPI_CSR[0] = SPI_CSR_BITS_8_BIT | mode_config | SPI_CSR_SCBR(1);

	interface->SPI_CR = SPI_CR_SPIEN; // Enable SPI
}

struct spi_device_t spi_init_device(enum spi_interface_no interface_no, struct pio_pin_t cs_pio, u8 cs_inverted)
{
	void *interface;

	switch (interface_no) {
		case SPI_INTERFACE_0:
			interface = (void *) SPI0;
			break;

#if defined(SPI1)
		case SPI_INTERFACE_1:
			interface = (void *) SPI1;
			break;
#endif
	}

	struct spi_device_t device = {
		.interface = interface,
		.cs_pio = cs_pio,
		.cs_inverted = !!cs_inverted,
	};

	return device;
}

void spi_select(struct spi_device_t device, u8 state)
{
	/* state 1 = selected, active low unless cs_inverted is set */
	pio_set(device.cs_pio, (!state) ^ device.cs_inverted);
}

u8 spi_transfer_byte(struct spi_device_t device, const u8 data)
{
	/* wait for transmit buffer empty */
	while (!(((Spi *) device.interface)->SPI_SR & SPI_SR_TXEMPTY_Msk)) continue;

	((Spi *) device.interface)->SPI_TDR = data & 0xFF;

	/* wait for transmission done */
	while (!(((Spi *) device.interface)->SPI_SR & SPI_SR_TXEMPTY_Msk)) continue;

	return (u8) (((Spi *) device.interface)->SPI_RDR & 0xFF);
}

void spi_transfer(struct spi_device_t device, const u8 *src, u32 size, u8 *dst)
{
	if (src) {
		while (size--) {
			if (dst)
				*(dst++) = spi_transfer_byte(device, *(src++));
			else
				spi_transfer_byte(device, *(src++));
		}
	} else if (dst) {
		while (size--) *(dst++) = spi_transfer_byte(device, 0x00);
	}
}
