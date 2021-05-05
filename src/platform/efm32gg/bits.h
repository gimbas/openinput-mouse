/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#pragma once

#define SRAM_BIT_ADDR(addr, bit)     (BITBAND_RAM_BASE + ((uint32_t)(addr) -SRAM_BASE) * 32 + (bit) *4)
#define SRAM_BIT(addr, bit)	     *(volatile uint32_t *) SRAM_BIT_ADDR(addr, bit)
#define PERI_REG_BIT_ADDR(reg, bit)  (BITBAND_PER_BASE + ((uint32_t)(reg) -PER_MEM_BASE) * 32 + (bit) *4)
#define PERI_REG_BIT(reg, bit)	     *(volatile uint32_t *) PERI_REG_BIT_ADDR(reg, bit)
#define PERI_REG_BIT_SET_ADDR(reg)   (PER_BITSET_MEM_BASE + ((uint32_t)(reg) -PER_MEM_BASE))
#define PERI_REG_BIT_SET(reg)	     *(volatile uint32_t *) PERI_REG_BIT_SET_ADDR(reg)
#define PERI_REG_BIT_CLEAR_ADDR(reg) (PER_BITCLR_MEM_BASE + ((uint32_t)(reg) -PER_MEM_BASE))
#define PERI_REG_BIT_CLEAR(reg)	     *(volatile uint32_t *) PERI_REG_BIT_CLEAR_ADDR(reg)
