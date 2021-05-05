/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#include <em_device.h>

#include "util/data.h"
#include "util/hid_descriptors.h"
#include "util/keycodes.h"
#include "util/types.h"

#include "platform/efm32gg/cmu.h"
#include "platform/efm32gg/emu.h"
#include "platform/efm32gg/gpio.h"
#include "platform/efm32gg/hal/hid.h"
#include "platform/efm32gg/systick.h"
#include "platform/efm32gg/usb.h"

#include "keyboard_engine/keyboard_engine.h"
#include "platform/efm32gg/driver/key_matrix.h"

#define CFG_TUSB_CONFIG_FILE "targets/efm32gg12-generic/tusb_config.h"
#include "tusb.h"

void main()
{
#if defined(DCDC_PRESENT) && defined(DCDC_ENABLE)
	emu_dcdc_init(DCDC_VOLTAGE, DCDC_ACTIVE_I, DCDC_SLEEP_I, DCDC_REVERSE_I); // Init DC-DC converter
	emu_init(0);
#else
	emu_init(1);
#endif

	emu_reg_init(REGULATOR_OUT); // set internal regulator voltage

	cmu_hfxo_startup_calib(0x200, 0x145); // Config HFXO Startup for 1280 uA, 36 pF (18 pF + 2 pF CLOAD)
	cmu_hfxo_steady_calib(0x009, 0x145); // Config HFXO Steady for 12 uA, 36 pF (18 pF + 2 pF CLOAD)

	cmu_init(EXTERNAL_CLOCK_VALUE); // Init Clock Management Unit
	cmu_update_clock_tree();

	cmu_ushfrco_calib(1, USHFRCO_CALIB_48M, 48000000); // Enable and calibrate USHFRCO for 48 MHz
	cmu_auxhfrco_calib(1, AUXHFRCO_CALIB_32M, 32000000); // Enable and calibrate AUXHFRCO for 32 MHz

	systick_init(); // Init system tick

	struct gpio_config_t gpio_config;

	struct gpio_pin_t led_io = {.port = GPIO_PORT_A, .pin = 12};

	struct gpio_pin_t col_ios[] = {
		{.port = GPIO_PORT_E, .pin = 11}, /* 0 */
		{.port = GPIO_PORT_E, .pin = 12}, /* 1 */
		{.port = GPIO_PORT_E, .pin = 13}, /* 2 */
		{.port = GPIO_PORT_E, .pin = 14}, /* 3 */
		{.port = GPIO_PORT_E, .pin = 15}, /* 4 */
		{.port = GPIO_PORT_A, .pin = 15}, /* 5 */
		{.port = GPIO_PORT_A, .pin = 0}, /* 6 */
		{.port = GPIO_PORT_A, .pin = 1}, /* 7 */
		{.port = GPIO_PORT_A, .pin = 2}, /* 8 */
		{.port = GPIO_PORT_A, .pin = 3}, /* 9 */
		{.port = GPIO_PORT_A, .pin = 4}, /* 10 */
		{.port = GPIO_PORT_A, .pin = 5}, /* 11 */
		{.port = GPIO_PORT_A, .pin = 6}, /* 12 */
		{.port = GPIO_PORT_B, .pin = 3}, /* 13 */
		{.port = GPIO_PORT_B, .pin = 4}, /* 14 */
		{.port = GPIO_PORT_B, .pin = 5}, /* 15 */
		{.port = GPIO_PORT_B, .pin = 6}, /* 16 */
		{.port = GPIO_PORT_C, .pin = 4}, /* 17 */
		{.port = GPIO_PORT_C, .pin = 5}, /* 18 */
		{.port = GPIO_PORT_A, .pin = 8}, /* 19 */
		{.port = GPIO_PORT_A, .pin = 12}, /* 20 */
	};

	struct gpio_pin_t row_ios[] = {
		{.port = GPIO_PORT_A, .pin = 13}, /* 0 */
		{.port = GPIO_PORT_A, .pin = 14}, /* 1 */
		{.port = GPIO_PORT_B, .pin = 11}, /* 2 */
		{.port = GPIO_PORT_B, .pin = 12}, /* 3 */
		{.port = GPIO_PORT_D, .pin = 0}, /* 4 */
		{.port = GPIO_PORT_D, .pin = 1}, /* 5 */
		{.port = GPIO_PORT_D, .pin = 2}, /* 6 */
		{.port = GPIO_PORT_D, .pin = 3}, /* 7 */
		{.port = GPIO_PORT_D, .pin = 4}, /* 8 */
		{.port = GPIO_PORT_D, .pin = 5}, /* 9 */
		{.port = GPIO_PORT_D, .pin = 6}, /* 10 */
		{.port = GPIO_PORT_D, .pin = 8}, /* 11 */
	};

	gpio_init_config(&gpio_config);

	gpio_setup_pin(&gpio_config, led_io, GPIO_MODE_WIREDAND, 0);

	for (size_t i = 0; i < 21; i++) gpio_setup_pin(&gpio_config, col_ios[i], GPIO_MODE_PUSHPULL, 0);

	for (size_t i = 0; i < 12; i++) gpio_setup_pin(&gpio_config, row_ios[i], GPIO_MODE_INPUTPULLFILTER, 0);

	gpio_apply_config(gpio_config);

	/* key engine */
	struct key_t key_data[16 * 6] = {};

	struct key_matrix_t key_matrix = key_matrix_init(col_ios, 16, row_ios, 6, key_data, 1);

	struct key_t *key_list[16 * 6] = {};

	for (u16 i = 0; i < 16 * 6; i++) key_list[i] = &key_data[i];

	struct layer_node_t layer0[16 * 6] = {
		{.node_function = layer_node_regular, .data[0] = keycode_A},
		{.node_function = layer_node_regular, .data[0] = keycode_B},
		{.node_function = layer_node_regular, .data[0] = keycode_C},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
		{.node_function = layer_node_none},
	};
	struct layer_node_t active_layer[16 * 6];
	struct layer_node_t *layer_list = layer0;

	struct keyboard_engine_config_t keyboard_engine_config = {
		.key_list = key_list,
		.key_list_size = 16 * 6,
		.layer_list = &layer_list,
		.layer_list_size = 1,
		.active_layer = active_layer,
		.tap_interval = 200,
	};

	keyboard_engine_load_layer(keyboard_engine_config, 0);

	struct hid_hal_t hid_hal;
	u8 info_functions[] = {
		OI_FUNCTION_VERSION,
		OI_FUNCTION_FW_INFO,
		OI_FUNCTION_SUPPORTED_FUNCTION_PAGES,
		OI_FUNCTION_SUPPORTED_FUNCTIONS,
	};

	/* create protocol config */
	struct protocol_config_t protocol_config;
	memset(&protocol_config, 0, sizeof(protocol_config));
	protocol_config.device_name = "openinput Device";
	protocol_config.hid_hal = hid_hal_init();
	protocol_config.functions[INFO] = info_functions;
	protocol_config.functions_size[INFO] = sizeof(info_functions);

	usb_attach_protocol_config(protocol_config);

	usb_init();

	struct keyboard_report_t keyboard_report;
	struct keyboard_engine_return_t keyboard_engine_return;

	for (;;) {
		tud_task();

		key_matrix_scan(&key_matrix);
		keyboard_engine_return = keyboard_engine_update(keyboard_engine_config);

		if (tud_hid_n_ready(2)) {
			static u8 last_report_had_data;

			//if(keyboard_engine_return.data_present || last_report_had_data)
			{
				memset(&keyboard_report, 0, sizeof(keyboard_report));

				keyboard_report.id = KEYBOARD_REPORT_ID;
				//for(size_t i = 0; i < 6; i++)
				//	keyboard_report.keys[i] = keyboard_engine_return.keycodes[i];
				keyboard_report.keys[0] = 0x0A;

				last_report_had_data = keyboard_engine_return.data_present;

				tud_hid_n_report(2, 0, &keyboard_report, sizeof(keyboard_report));
			}
		}
	}
}
