
#include "platform/efm32gg/ldma.h"
#include "platform/efm32gg/atomic.h"
#include "platform/efm32gg/bits.h"
#include "platform/efm32gg/nvic.h"
#include "util/data.h"
#include <em_device.h>
#include <stdlib.h>

static ldma_ch_isr_t channel_isr[DMA_CHAN_COUNT];

void _ldma_isr()
{
	u32 flags = LDMA->IFC;

	if (flags & LDMA_IFC_ERROR) {
		u8 error_ch = (LDMA->STATUS & _LDMA_STATUS_CHERROR_MASK) << _LDMA_STATUS_CHERROR_SHIFT;

		if (channel_isr[error_ch])
			channel_isr[error_ch](1);
	}

	for (u8 channel = 0; channel < DMA_CHAN_COUNT; channel++) {
		if (!(flags & BIT(channel)))
			continue;

		if (channel_isr[channel])
			channel_isr[channel](0);
	}
}

void ldma_init()
{
	CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_LDMA;

	LDMA->IFC = _LDMA_IFC_MASK; /* Clear all flags */
	IRQ_CLEAR(LDMA_IRQn); /* Clear pending vector */
	IRQ_SET_PRIO(LDMA_IRQn, 2, 0); /* Set priority 2,0 */
	IRQ_ENABLE(LDMA_IRQn); /* Enable vector */
	LDMA->IEN = _LDMA_IEN_MASK; /* Enable all channel flags */
}

void ldma_sync_set(u8 mask)
{
	PERI_REG_BIT_SET(&(LDMA->SYNC)) = mask;
}
void ldma_sync_clear(u8 mask)
{
	PERI_REG_BIT_CLEAR(&(LDMA->SYNC)) = mask;
}

void ldma_ch_config(u8 channel, u32 source, u32 src_inc_sign, u32 dst_inc_sign, u32 arbitration_slots, u8 loop_count)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	LDMA->CH[channel].REQSEL = source;
	LDMA->CH[channel].CFG = dst_inc_sign | src_inc_sign | arbitration_slots;
	LDMA->CH[channel].LOOP = loop_count;
}
void ldma_ch_set_isr(u8 channel, ldma_ch_isr_t isr)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	channel_isr[channel] = isr;
}
void ldma_ch_load(u8 channel, struct ldma_descriptor_t *descriptor)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	if (!descriptor)
		return;

	if ((u32) descriptor & 3) /* Descriptors must be word aligned */
		return;

	LDMA->CH[channel].LINK = (u32) descriptor | LDMA_CH_LINK_LINK;

	PERI_REG_BIT_CLEAR(&(LDMA->CHDONE)) = BIT(channel);
	LDMA->LINKLOAD = BIT(channel);
}
void ldma_ch_sw_req(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	LDMA->SWREQ = BIT(channel);
}
void ldma_ch_enable(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	PERI_REG_BIT_SET(&(LDMA->CHEN)) = BIT(channel);
}
void ldma_ch_disable(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	PERI_REG_BIT_CLEAR(&(LDMA->CHEN)) = BIT(channel);
}
void ldma_ch_peri_req_enable(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	PERI_REG_BIT_CLEAR(&(LDMA->REQDIS)) = BIT(channel);
}
void ldma_ch_peri_req_disable(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	PERI_REG_BIT_SET(&(LDMA->REQDIS)) = BIT(channel);
}
void ldma_ch_req_clear(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return;

	LDMA->REQCLEAR = BIT(channel);
}
u8 ldma_ch_get_busy(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return 0;

	return PERI_REG_BIT(&(LDMA->CHBUSY), channel);
}
u16 ldma_ch_get_remaining_xfers(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return 0;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		u16 usRemaining = (LDMA->CH[channel].CTRL & _LDMA_CH_CTRL_XFERCNT_MASK) >> _LDMA_CH_CTRL_XFERCNT_SHIFT;

		if (PERI_REG_BIT(&(LDMA->CHDONE), channel) || (!usRemaining && (LDMA->IF & BIT(channel))))
			return 0;

		return usRemaining + 1;
	}
}
void *ldma_ch_get_next_src_addr(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return NULL;

	return (void *) (LDMA->CH[channel].SRC);
}
void *ldma_ch_get_next_dst_addr(u8 channel)
{
	if (channel >= DMA_CHAN_COUNT)
		return NULL;

	return (void *) (LDMA->CH[channel].DST);
}
