/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#pragma once

#include "platform/samx7x/pio.h"
#include "util/types.h"

enum spi_mode {
	SPI_MODE0 = 0,
	SPI_MODE1 = 1,
	SPI_MODE2 = 2,
	SPI_MODE3 = 3,
};

enum spi_interface_no {
	SPI_INTERFACE_0,
#if defined(SPI1)
	SPI_INTERFACE_1,
#endif
};

struct spi_device_t {
	struct pio_pin_t cs_pio;
	u8 cs_inverted;
	void *interface;
};

void spi_init_interface(enum spi_interface_no interface_no, enum spi_mode mode, u32 frequency);
struct spi_device_t spi_init_device(enum spi_interface_no interface_no, struct pio_pin_t cs_pio, u8 cs_inverted);
void spi_select(struct spi_device_t device, u8 state);
u8 spi_transfer_byte(struct spi_device_t device, const u8 data);
void spi_transfer(struct spi_device_t device, const u8 *src, u32 size, u8 *dst);
