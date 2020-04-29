// SPDX-License-Identifier: MIT

#include "profile.h"

//--------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------

list_t* profile_list = NULL;

//--------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------

void load_profiles()
{
	if(profile_list)
		list_destroy(profile_list);

	profile_list = list_new();

	if(!profile_list)
		return;

	target_load_profiles(profile_list);

	profile_key_event();
}

void unload_profiles()
{
	list_destroy(profile_list);
}

void save_profiles()
{
	if(!profile_list)
		return;

	target_save_profiles(profile_list);
}

void add_profile(profile_t profile)
{
	if(!profile_list)
		profile_list = list_new();

	profile_t* new_profile = (profile_t*)malloc(sizeof(profile_t));

	if(!profile_list || !new_profile)
		return;

	*new_profile = profile;

	list_rpush(profile_list, list_node_new(new_profile));
}

void profile_key_event()
{
	if(!profile_list)
		return;

	list_node_t* profile_node;

	static list_iterator_t* profile_list_iterator = NULL;

	if(!(profile_node = list_iterator_next(profile_list_iterator)))
	{
		if(!profile_list_iterator)
		{
			if(!(profile_list_iterator = list_iterator_new_from_node(list_at(profile_list, target_get_default_profile_index()), LIST_HEAD)))
				profile_list_iterator = list_iterator_new(profile_list, LIST_HEAD);
		}
		else
		{
			list_iterator_destroy(profile_list_iterator);
			profile_list_iterator = list_iterator_new(profile_list, LIST_HEAD);
		}

		profile_node = list_iterator_next(profile_list_iterator);

		if(!profile_node)
			return;
	}

	profile_t* current_profile = (profile_t*)(profile_node->val);

	target_set_dpi(current_profile->dpi);
	target_leds_write(current_profile->color.r, current_profile->color.g, current_profile->color.b);
}