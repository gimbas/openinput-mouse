// SPDX-License-Identifier: MIT

#ifndef __PROFILE_H__
#define __PROFILE_H__

#include "list.h"
#include "target.h"

//--------------------------------------------------------------------
// Data types
//--------------------------------------------------------------------

typedef struct
{
	uint16_t dpi;
	struct
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	} color;
} profile_t;


//--------------------------------------------------------------------
// Prototypes
//--------------------------------------------------------------------

void load_profiles();

void unload_profiles();

void save_profiles();

void profile_key_event();

#endif // __PROFILE_H__
