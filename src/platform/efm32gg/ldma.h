/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#pragma once

#include "util/types.h"

typedef void (*ldma_ch_isr_t)(u8);

struct ldma_descriptor_t {
	volatile u32 CTRL;
	union {
		volatile void *volatile SRC;
		volatile u32 IMMVAL;
		volatile u16 SYNC;
	};

	union {
		volatile void *volatile DST;
		volatile u16 MATCH;
	};

	volatile u32 LINK;
} __attribute__((aligned(4)));

void ldma_init();

void ldma_sync_set(u8 mask);
void ldma_sync_clear(u8 mask);

void ldma_ch_config(u8 channel, u32 source, u32 src_inc_sign, u32 dst_inc_sign, u32 arbitration_slots, u8 loop_count);
void ldma_ch_set_isr(u8 channel, ldma_ch_isr_t isr);
void ldma_ch_load(u8 channel, struct ldma_descriptor_t *descriptor);
void ldma_ch_sw_req(u8 channel);
void ldma_ch_enable(u8 channel);
void ldma_ch_disable(u8 channel);
void ldma_ch_peri_req_enable(u8 channel);
void ldma_ch_peri_req_disable(u8 channel);
void ldma_ch_req_clear(u8 channel);
u8 ldma_ch_get_busy(u8 channel);
u16 ldma_ch_get_remaining_xfers(u8 channel);
void *ldma_ch_get_next_src_addr(u8 channel);
void *ldma_ch_get_next_dst_addr(u8 channel);
