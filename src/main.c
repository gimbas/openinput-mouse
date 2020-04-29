// SPDX-License-Identifier: MIT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "target.h"
#include "profile.h"

#include "usb.h"
#include "tusb.h"

//--------------------------------------------------------------------
// Enums and data types
//--------------------------------------------------------------------

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 500,
  BLINK_MOUNTED = 250,
  BLINK_SUSPENDED = 2500,
};

//--------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

//--------------------------------------------------------------------
// Function declaration
//--------------------------------------------------------------------

void leds_task(void);
void hid_task(void);

//--------------------------------------------------------------------
// Functions
//--------------------------------------------------------------------

/*------------- MAIN -------------*/
int main(void)
{
  tusb_init();

  while (1)
  {
    target_tasks();

    hid_task();

    tud_task(); // tinyusb device task

    leds_task();
  }

  return 0;
}
