/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#pragma once

#include "platform/efm32gg/ldma.h"
#include "util/types.h"

// #define WS2812B_T            800000UL
// #define WS2812B_T0H             0.000000400f // 400 ns
// #define WS2812B_T1H             0.000000800f // 800 ns

/* SK6812 */
#define WS2812B_T   0.000001200f // 1.25 us
#define WS2812B_T0H 0.000000320f // 320 ns
#define WS2812B_T1H 0.000000640f // 640 ns

enum timer_no {
	timer0,
	timer1,
	timer2,
	timer3,
};

struct color_t {
	u8 red, u8 green, u8 blue,
}

struct led_string {
	u16 string_length;
	struct color_t led_string *;
	u16 buffer[3 * 8];
	u8 dma_channel;
	struct ldma_descriptor_t dma_descriptor[4];
	TIMER_TypeDef *timer;
	u8 cc_channel;
	u16 t0h;
	u16 t1h;
};

struct led_string led_init(u16 string_length, enum timer_no timer, u8 cc_channel, u8 timer_location, u8 dma_channel);
void led_set_color(struct led_string string, u16 led_no, u8 red, u8 green, u8 blue);
void led_update(struct led_string string);
