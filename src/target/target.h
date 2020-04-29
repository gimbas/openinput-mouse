// SPDX-License-Identifier: MIT

#ifndef _TARGET_H_
#define _TARGET_H_

#include "tusb.h"

#include "list.h"
#include "profile.h"

//--------------------------------------------------------------------
// Data types
//--------------------------------------------------------------------

typedef struct {
    int16_t dx;
    int16_t dy;
} deltas_t;

//--------------------------------------------------------------------
// profiles API
//--------------------------------------------------------------------

void target_load_profiles(list_t* profile_list);
void target_save_profiles(list_t* profile_list);
uint8_t target_get_default_profile_index();
void target_set_default_profile_index(uint8_t index);

void target_set_dpi(uint16_t dpi);

//--------------------------------------------------------------------
// Target Porting API
//--------------------------------------------------------------------

void target_tasks();

uint64_t target_ticks();

void target_delay_ms(uint16_t ticks);

//--------------------------------------------------------------------
// Outputs API
//--------------------------------------------------------------------

void target_leds_write(uint8_t r, uint8_t g, uint8_t b);

//--------------------------------------------------------------------
// Inputs API
//--------------------------------------------------------------------

deltas_t target_sensor_deltas_get();

uint8_t target_mouse_btns_get();

int8_t target_wheel_get();

#endif // _TARGET_H_