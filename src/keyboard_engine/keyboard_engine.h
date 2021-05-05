/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#pragma once

#include "util/types.h"

enum key_status_t {
	key_released,
	key_pressed,
};

struct key_t {
	u16 key_status : 1;
	u16 key_t_delta : 15;
};

enum layer_node_functions_t {
	layer_node_none,
	layer_node_regular,
	layer_node_tap_hold,
	layer_node_set_layer,
	layer_node_mouse_buttons,
};

struct layer_node_t {
	u8 node_function;
	u8 data[3];
};

struct keyboard_engine_config_t {
	struct key_t **key_list;
	u8 key_list_size;
	struct layer_node_t **layer_list;
	u8 layer_list_size;
	struct layer_node_t *active_layer;
	u16 tap_interval;
};

struct keyboard_engine_return_t {
	u8 data_present;
	u8 keycodes[6];
	u8 mouse_buttons;
};

struct keyboard_engine_return_t keyboard_engine_update(struct keyboard_engine_config_t config);
void keyboard_engine_load_layer(struct keyboard_engine_config_t config, u8 layer_no);
