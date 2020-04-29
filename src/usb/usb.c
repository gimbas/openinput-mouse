// SPDX-License-Identifier: MIT

#include "usb.h"

//--------------------------------------------------------------------
// USB device callbacks
//--------------------------------------------------------------------

// Invoked when device is mounted
void tud_mount_cb(void)
{

}

// Invoked when device is unmounted
void tud_umount_cb(void)
{

}

// Invoked when usb bus is suspended
void tud_suspend_cb(bool remote_wakeup_en)
{
	(void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{

}

/*------------- USB HID -------------*/

void hid_task()
{
	/*------------- Mouse -------------*/
	if(tud_hid_ready())
	{
		//
		// Wheel Mouse
		//
		// Input report - 5 bytes
		//
		//     Byte | D7      D6      D5      D4      D3      D2      D1      D0
		//    ------+---------------------------------------------------------------------
		//      0   |  0       0       0    Forward  Back    Middle  Right   Left (Button)
		//      1   |                             X
		//      2   |                             Y
		//      3   |                       Vertical Wheel
		//      4   |                    Horizontal (Tilt) Wheel
		//

		uint8_t buttons = target_mouse_btns_get();
		deltas_t deltas = target_sensor_deltas_get();
		int8_t vertical = target_wheel_get();

		typedef struct __attribute__ ((packed))
		{
			uint8_t buttons; /**< buttons mask for currently pressed buttons in the mouse. */
			int16_t  x;       /**< Current delta x movement of the mouse. */
			int16_t  y;       /**< Current delta y movement on the mouse. */
			int8_t  wheel;   /**< Current delta wheel movement on the mouse. */
			int8_t  pan;     // using AC Pan
		} hid_mouse_alt_report_t;

		hid_mouse_alt_report_t report =
		{
			.buttons = buttons,
			.x       = deltas.dx,
			.y       = deltas.dy,
			.wheel   = vertical,
			.pan     = 0
		};

		tud_hid_report(REPORT_ID_MOUSE, &report, sizeof(report));
	}
}


// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) reqlen;

	return 0;
}

// Invoked when received SET_REPORT control request
void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) bufsize;
}