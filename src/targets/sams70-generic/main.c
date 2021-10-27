/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#include "util/data.h"
#include "util/types.h"

#include "platform/samx7x/eefc.h"
#include "platform/samx7x/hal/hid.h"
#include "platform/samx7x/hal/spi.h"
#include "platform/samx7x/pio.h"
#include "platform/samx7x/pmc.h"
#include "platform/samx7x/spi.h"
#include "platform/samx7x/systick.h"
#include "platform/samx7x/usb.h"
#include "platform/samx7x/wdt.h"

#include "protocol/protocol.h"

#define CFG_TUSB_CONFIG_FILE "targets/sams70-generic/tusb_config.h"
#include "tusb.h"

void main()
{
	wdt_disable();

	eefc_tcm_disable();

	pmc_init(EXTERNAL_CLOCK_VALUE, 0UL);
	pmc_init_usb();
	pmc_update_clock_tree();

	systick_init();

	pio_init();

	struct pio_pin_t cs_io = {.port = PIO_PORT_A, .pin = 10};
	struct pio_pin_t miso_io = {.port = PIO_PORT_A, .pin = 12};
	struct pio_pin_t mosi_io = {.port = PIO_PORT_A, .pin = 13};
	struct pio_pin_t sclk_io = {.port = PIO_PORT_B, .pin = 13};

	struct pio_config_t io_conf = PIO_DEFAULT_CONFIG;

	io_conf.direction = PIO_DIRECTION_OUT;
	io_conf.high_drive = 1;
	io_conf.pull = PIO_PULL_UP;
	io_conf.peripheral_control = 0;
	io_conf.mux = PIO_MUX_A;

	pio_config(cs_io, io_conf);

	io_conf.peripheral_control = 1;

	pio_config(mosi_io, io_conf);
	pio_config(sclk_io, io_conf);

	io_conf.direction = PIO_DIRECTION_IN;

	pio_config(miso_io, io_conf);

	qspi_init_interface(SPI_MODE3, 1000000);

	struct qspi_device_t sensor_spi_device = qspi_init_device(cs_io, 0);

	// struct hid_hal_t hid_hal;
	// u8 info_functions[] = {
	// 	OI_FUNCTION_VERSION,
	// 	OI_FUNCTION_FW_INFO,
	// 	OI_FUNCTION_SUPPORTED_FUNCTION_PAGES,
	// 	OI_FUNCTION_SUPPORTED_FUNCTIONS,
	// };

	// /* create protocol config */
	// struct protocol_config_t protocol_config;
	// memset(&protocol_config, 0, sizeof(protocol_config));
	// protocol_config.device_name = "openinput Device";
	// protocol_config.hid_hal = hid_hal_init();
	// protocol_config.functions[INFO] = info_functions;
	// protocol_config.functions_size[INFO] = sizeof(info_functions);

	// usb_attach_protocol_config(protocol_config);

	// usb_init();

	for (;;) {
		// tud_task();
		qspi_transfer_byte(sensor_spi_device, 0x55);
		delay_ms(500);
	}
}
