/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#pragma once

#include "util/types.h"

/* ports */
enum pio_port_t {
	PIO_PORT_A = 0,
	PIO_PORT_B = 1,
	PIO_PORT_C = 2,
	PIO_PORT_D = 3,
	PIO_PORT_E = 4,
};

struct pio_pin_t {
	u8 port;
	u8 pin;
};

enum pio_mux_t {
	PIO_MUX_A = 0,
	PIO_MUX_B = 1,
	PIO_MUX_C = 2,
	PIO_MUX_D = 3,
};

enum pio_pull_t {
	PIO_PULL_NONE = 0,
	PIO_PULL_UP = 1,
	PIO_PULL_DOWN = 2,
};

enum pio_direction_t {
	PIO_DIRECTION_OUT = 0,
	PIO_DIRECTION_IN = 1,
};
struct pio_config_t {
	enum pio_direction_t direction;
	u8 open_drain;
	u8 out_data;
	u8 high_drive;
	enum pio_pull_t pull;
	u8 filter;
	u8 slow_filter;
	u8 peripheral_control;
	enum pio_mux_t mux;
};

#define PIO_DEFAULT_CONFIG                                                                                             \
	{                                                                                                              \
		.direction = PIO_DIRECTION_IN, .open_drain = 0, .out_data = 0, .high_drive = 0, .pull = PIO_PULL_DOWN, \
		.filter = 0, .slow_filter = 0, .peripheral_control = 0, .mux = PIO_MUX_A,                              \
	}

void pio_init();

void pio_config(struct pio_pin_t pin, struct pio_config_t config);

void pio_peripheral_mux(struct pio_pin_t pin, enum pio_mux_t mux);

void pio_peripheral_control(struct pio_pin_t pin, u8 enable);

void pio_direction(struct pio_pin_t pin, enum pio_direction_t direction);

void pio_pull(struct pio_pin_t pin, enum pio_pull_t pull);

void pio_set(struct pio_pin_t pin, u8 state);

u8 pio_get(struct pio_pin_t pin);
