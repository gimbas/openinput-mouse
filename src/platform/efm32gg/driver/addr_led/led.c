/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#include <em_device.h>
#include <stdlib.h>
#include <string.h>

#include "util/data.h"

#include "platform/efm32gg/bits.h"
#include "platform/efm32gg/cmu.h"
#include "platform/efm32gg/driver/addr_led/led.h"

struct led_string
led_init(struct color_t *leds_string, u16 string_length, enum timer_no timer, u8 cc_channel, u8 cc_location, u8 dma_channel)
{
	volatile struct led_string string;
	u32 dma_src;

	if (!leds_string)
		return string;

	if (cc_location > 7 || cc_channel > 2)
		return string;

	switch (timer) {
		case timer0:
			CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER0;
			string.timer = TIMER0;
			dma_src = LDMA_CH_REQSEL_SOURCESEL_TIMER0 | LDMA_CH_REQSEL_SIGSEL_TIMER0UFOF;
			break;

		case timer1:
			CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER1;
			string.timer = TIMER1;
			dma_src = LDMA_CH_REQSEL_SOURCESEL_TIMER1 | LDMA_CH_REQSEL_SIGSEL_TIMER1UFOF;
			break;

		case timer2:
			CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER2;
			string.timer = TIMER2;
			dma_src = LDMA_CH_REQSEL_SOURCESEL_TIMER2 | LDMA_CH_REQSEL_SIGSEL_TIMER2UFOF;
			break;

		case timer3:
			CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER3;
			string.timer = TIMER3;
			dma_src = LDMA_CH_REQSEL_SOURCESEL_TIMER3 | LDMA_CH_REQSEL_SIGSEL_TIMER3UFOF;
			break;

		default:
			return string;
			break;
	}

	string.string_length = string_length;
	string.dma_channel = dma_channel;
	string.cc_channel = cc_channel;
	string.led_string = leds_string;

	if (!string.buffer)
		return string;

	memset(string.buffer, 0, string_length * 3 * 8 * sizeof(u16));

	struct cmu_clock_tree_t clock_tree = cmu_get_clock_tree();

	string.timer->CTRL = TIMER_CTRL_RSSCOIST | TIMER_CTRL_PRESC_DIV1 | TIMER_CTRL_CLKSEL_PRESCHFPERCLK |
			     TIMER_CTRL_FALLA_NONE | TIMER_CTRL_RISEA_NONE | TIMER_CTRL_DMACLRACT | TIMER_CTRL_MODE_UP;
	string.timer->TOP = (clock_tree.hfper_freq * WS2812B_T) - 1;
	string.timer->CNT = 0x0000;

	string.timer->CC[cc_channel].CTRL = TIMER_CC_CTRL_PRSCONF_LEVEL | TIMER_CC_CTRL_CUFOA_NONE | TIMER_CC_CTRL_COFOA_SET |
					    TIMER_CC_CTRL_CMOA_CLEAR | TIMER_CC_CTRL_OUTINV | TIMER_CC_CTRL_MODE_OUTPUTCOMPARE;

	switch (cc_channel) {
		case 0:
			string.timer->ROUTELOC0 = (u32) cc_location << _TIMER_ROUTELOC0_CC0LOC_SHIFT;
			string.timer->ROUTEPEN |= TIMER_ROUTEPEN_CC0PEN;
			break;

		case 1:
			string.timer->ROUTELOC0 = (u32) cc_location << _TIMER_ROUTELOC0_CC1LOC_SHIFT;
			string.timer->ROUTEPEN |= TIMER_ROUTEPEN_CC1PEN;
			break;

		case 2:
			string.timer->ROUTELOC0 = (u32) cc_location << _TIMER_ROUTELOC0_CC2LOC_SHIFT;
			string.timer->ROUTEPEN |= TIMER_ROUTEPEN_CC2PEN;
			break;
	}

	ldma_ch_disable(dma_channel);
	ldma_ch_peri_req_disable(dma_channel);
	ldma_ch_req_clear(dma_channel);

	ldma_ch_config(
		dma_channel,
		dma_src,
		LDMA_CH_CFG_SRCINCSIGN_POSITIVE,
		LDMA_CH_CFG_DSTINCSIGN_DEFAULT,
		LDMA_CH_CFG_ARBSLOTS_DEFAULT,
		0);

	string.dma_descriptor[0].CTRL = LDMA_CH_CTRL_DSTMODE_ABSOLUTE | LDMA_CH_CTRL_SRCMODE_ABSOLUTE |
					LDMA_CH_CTRL_DSTINC_NONE | LDMA_CH_CTRL_SIZE_HALFWORD | LDMA_CH_CTRL_SRCINC_NONE |
					LDMA_CH_CTRL_REQMODE_BLOCK | LDMA_CH_CTRL_BLOCKSIZE_UNIT1 |
					((0 << _LDMA_CH_CTRL_XFERCNT_SHIFT) & _LDMA_CH_CTRL_XFERCNT_MASK) |
					LDMA_CH_CTRL_STRUCTREQ | LDMA_CH_CTRL_STRUCTTYPE_TRANSFER;
	string.dma_descriptor[0].SRC = &(string.timer->TOP);
	string.dma_descriptor[0].DST = &(string.timer->CNT);
	string.dma_descriptor[0].LINK = 0x00000010 | LDMA_CH_LINK_LINK | LDMA_CH_LINK_LINKMODE_RELATIVE;

	string.dma_descriptor[1].CTRL = LDMA_CH_CTRL_STRUCTTYPE_WRITE;
	string.dma_descriptor[1].IMMVAL = TIMER_CMD_START;
	string.dma_descriptor[1].DST = &(string.timer->CMD);
	string.dma_descriptor[1].LINK = 0x00000010 | LDMA_CH_LINK_LINK | LDMA_CH_LINK_LINKMODE_RELATIVE;

	string.dma_descriptor[2].CTRL =
		LDMA_CH_CTRL_DSTMODE_ABSOLUTE | LDMA_CH_CTRL_SRCMODE_ABSOLUTE | LDMA_CH_CTRL_DSTINC_NONE |
		LDMA_CH_CTRL_SIZE_HALFWORD | LDMA_CH_CTRL_SRCINC_ONE | LDMA_CH_CTRL_REQMODE_BLOCK |
		LDMA_CH_CTRL_BLOCKSIZE_UNIT1 |
		((((string_length * 3 * 8) - 1) << _LDMA_CH_CTRL_XFERCNT_SHIFT) & _LDMA_CH_CTRL_XFERCNT_MASK) |
		LDMA_CH_CTRL_STRUCTREQ | LDMA_CH_CTRL_STRUCTTYPE_TRANSFER;
	string.dma_descriptor[2].SRC = string.buffer;
	string.dma_descriptor[2].DST = &(string.timer->CC[cc_channel].CCV);
	string.dma_descriptor[2].LINK = 0x00000010 | LDMA_CH_LINK_LINK | LDMA_CH_LINK_LINKMODE_RELATIVE;

	string.dma_descriptor[3].CTRL = LDMA_CH_CTRL_STRUCTTYPE_WRITE;
	string.dma_descriptor[3].IMMVAL = TIMER_CTRL_OSMEN;
	string.dma_descriptor[3].DST = (void *) PERI_REG_BIT_SET_ADDR(&(string.timer->CTRL));
	string.dma_descriptor[3].LINK = 0x00000000;

	ldma_ch_peri_req_enable(dma_channel);
	ldma_ch_enable(dma_channel);
}

void led_set_color(struct led_string string, u16 led_no, u8 red, u8 green, u8 blue)
{
	if (led_no >= string.string_length)
		return;

	u32 data = ((u32) green << 16) | ((u32) red << 8) | ((u32) blue << 0);
	u32 buff_offset = (u32) led_no * 3 * 8;

	u8 buff_count = 3 * 8;

	while (buff_count--) {
		string.buffer[buff_offset++] = (data & BIT(23)) ? string.t1h : string.t0h;

		data <<= 1;
	}
}

void led_update(struct led_string string)
{
	if (string.timer->STATUS & TIMER_STATUS_RUNNING)
		return;

	string.timer->CTRL &= ~TIMER_CTRL_OSMEN;
	string.timer->CC[string.cc_channel].CCV = string.timer->TOP - 1;

	ldma_ch_req_clear(string.dma_channel);
	ldma_ch_load(string.dma_channel, string.dma_descriptor);
}
