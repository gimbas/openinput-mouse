/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: 2021 Rafael Silva <perigoso@riseup.net>
 */

#include <em_device.h>

#include "util/data.h"
#include "util/reports.h"
#include "util/types.h"

#include "platform/efm32gg/cmu.h"
#include "platform/efm32gg/emu.h"
#include "platform/efm32gg/gpio.h"
#include "platform/efm32gg/hal/hid.h"
#include "platform/efm32gg/systick.h"
#include "platform/efm32gg/usb.h"

#define CFG_TUSB_CONFIG_FILE "targets/efm32gg12-generic/tusb_config.h"
#include "tusb.h"

enum keycodes {
	None = 0x00, /* No key pressed */
	ErrorRollover = 0x01, /* Keyboard Error Roll Over ("Phantom Key") */
	ErrorPostFail = 0x02, /* Keyboard POST Fail */
	ErrorUndefined = 0x03, /* Keyboard Error Undefined */
	KeyA = 0x04, /* Keyboard a and A */
	KeyB = 0x05, /* Keyboard b and B */
	KeyC = 0x06, /* Keyboard c and C */
	KeyD = 0x07, /* Keyboard d and D */
	KeyE = 0x08, /* Keyboard e and E */
	KeyF = 0x09, /* Keyboard f and F */
	KeyG = 0x0a, /* Keyboard g and G */
	KeyH = 0x0b, /* Keyboard h and H */
	KeyI = 0x0c, /* Keyboard i and I */
	KeyJ = 0x0d, /* Keyboard j and J */
	KeyK = 0x0e, /* Keyboard k and K */
	KeyL = 0x0f, /* Keyboard l and L */
	KeyM = 0x10, /* Keyboard m and M */
	KeyN = 0x11, /* Keyboard n and N */
	KeyO = 0x12, /* Keyboard o and O */
	KeyP = 0x13, /* Keyboard p and P */
	KeyQ = 0x14, /* Keyboard q and Q */
	KeyR = 0x15, /* Keyboard r and R */
	KeyS = 0x16, /* Keyboard s and S */
	KeyT = 0x17, /* Keyboard t and T */
	KeyU = 0x18, /* Keyboard u and U */
	KeyV = 0x19, /* Keyboard v and V */
	KeyW = 0x1a, /* Keyboard w and W */
	KeyX = 0x1b, /* Keyboard x and X */
	KeyY = 0x1c, /* Keyboard y and Y */
	KeyZ = 0x1d, /* Keyboard z and Z */
	Key1 = 0x1e, /* Keyboard 1 and ! */
	Key2 = 0x1f, /* Keyboard 2 and @ */
	Key3 = 0x20, /* Keyboard 3 and # */
	Key4 = 0x21, /* Keyboard 4 and $ */
	Key5 = 0x22, /* Keyboard 5 and % */
	Key6 = 0x23, /* Keyboard 6 and ^ */
	Key7 = 0x24, /* Keyboard 7 and &amp; */
	Key8 = 0x25, /* Keyboard 8 and * */
	Key9 = 0x26, /* Keyboard 9 and ( */
	Key0 = 0x27, /* Keyboard 0 and ) */
	KeyEnter = 0x28, /* Keyboard Return (ENTER) */
	KeyESC = 0x29, /* Keyboard ESCAPE */
	KeyBackspace = 0x2a, /* Keyboard DELETE (Backspace) */
	KeyTab = 0x2b, /* Keyboard Tab */
	KeySpace = 0x2c, /* Keyboard Spacebar */
	KeyMinus = 0x2d, /* Keyboard - and _ */
	KeyEqual = 0x2e, /* Keyboard = and + */
	KeyLeftBrace = 0x2f, /* Keyboard [ and { */
	KeyRightBrace = 0x30, /* Keyboard ] and } */
	KeyBackSlash = 0x31, /* Keyboard \ and | */
	KeyHashTilde = 0x32, /* Keyboard Non-US # and ~ */
	KeySemicolon = 0x33, /* Keyboard ; and : */
	KeyApostrophe = 0x34, /* Keyboard ' and " */
	KeyGrave = 0x35, /* Keyboard ` and ~ */
	KeyComma = 0x36, /* Keyboard , and LESSTHAN &lt; */
	KeyPeriod = 0x37, /* Keyboard . and > */
	KeyForwardSlash = 0x38, /* Keyboard / and ? */
	KeyCapslock = 0x39, /* Keyboard Caps Lock */
	KeyF1 = 0x3a, /* Keyboard F1 */
	KeyF2 = 0x3b, /* Keyboard F2 */
	KeyF3 = 0x3c, /* Keyboard F3 */
	KeyF4 = 0x3d, /* Keyboard F4 */
	KeyF5 = 0x3e, /* Keyboard F5 */
	KeyF6 = 0x3f, /* Keyboard F6 */
	KeyF7 = 0x40, /* Keyboard F7 */
	KeyF8 = 0x41, /* Keyboard F8 */
	KeyF9 = 0x42, /* Keyboard F9 */
	KeyF10 = 0x43, /* Keyboard F10 */
	KeyF11 = 0x44, /* Keyboard F11 */
	KeyF12 = 0x45, /* Keyboard F12 */
	KeyPrintScreen = 0x46, /* Keyboard Print Screen */
	KeyScrollLock = 0x47, /* Keyboard Scroll Lock */
	KeyPause = 0x48, /* Keyboard Pause */
	KeyInsert = 0x49, /* Keyboard Insert */
	KeyHome = 0x4a, /* Keyboard Home */
	KeyPageUp = 0x4b, /* Keyboard Page Up */
	KeyDelete = 0x4c, /* Keyboard Delete Forward */
	KeyEnd = 0x4d, /* Keyboard End */
	KeyPageDown = 0x4e, /* Keyboard Page Down */
	KeyRight = 0x4f, /* Keyboard Right Arrow */
	KeyLeft = 0x50, /* Keyboard Left Arrow */
	KeyDown = 0x51, /* Keyboard Down Arrow */
	KeyUp = 0x52, /* Keyboard Up Arrow */
	KeyNPNumlock = 0x53, /* Keyboard Num Lock and Clear */
	KeyNPSlash = 0x54, /* Keypad / */
	KeyNPAsterisk = 0x55, /* Keypad * */
	KeyNPMinus = 0x56, /* Keypad - */
	KeyNPPlus = 0x57, /* Keypad + */
	KeyNPEnter = 0x58, /* Keypad ENTER */
	KeyNP1 = 0x59, /* Keypad 1 and End */
	KeyNP2 = 0x5a, /* Keypad 2 and Down Arrow */
	KeyNP3 = 0x5b, /* Keypad 3 and PageDn */
	KeyNP4 = 0x5c, /* Keypad 4 and Left Arrow */
	KeyNP5 = 0x5d, /* Keypad 5 */
	KeyNP6 = 0x5e, /* Keypad 6 and Right Arrow */
	KeyNP7 = 0x5f, /* Keypad 7 and Home */
	KeyNP8 = 0x60, /* Keypad 8 and Up Arrow */
	KeyNP9 = 0x61, /* Keypad 9 and Page Up */
	KeyNP0 = 0x62, /* Keypad 0 and Insert */
	KeyNPDOT = 0x63, /* Keypad . and Delete */
	Key102ND = 0x64, /* Keyboard Non-US \ and | */
	KeyCompose = 0x65, /* Keyboard Application */
	KeyPower = 0x66, /* Keyboard Power */
	KeyNPEqual = 0x67, /* Keypad = */
	KeyF13 = 0x68, /* Keyboard F13 */
	KeyF14 = 0x69, /* Keyboard F14 */
	KeyF15 = 0x6a, /* Keyboard F15 */
	KeyF16 = 0x6b, /* Keyboard F16 */
	KeyF17 = 0x6c, /* Keyboard F17 */
	KeyF18 = 0x6d, /* Keyboard F18 */
	KeyF19 = 0x6e, /* Keyboard F19 */
	KeyF20 = 0x6f, /* Keyboard F20 */
	KeyF21 = 0x70, /* Keyboard F21 */
	KeyF22 = 0x71, /* Keyboard F22 */
	KeyF23 = 0x72, /* Keyboard F23 */
	KeyF24 = 0x73, /* Keyboard F24 */
	KeyOpen = 0x74, /* Keyboard Execute */
	KeyHelp = 0x75, /* Keyboard Help */
	KeyMenu = 0x76, /* Keyboard Menu */
	KeySelect = 0x77, /* Keyboard Select */
	KeyStop = 0x78, /* Keyboard Stop */
	KeyAgain = 0x79, /* Keyboard Again */
	KeyUndo = 0x7a, /* Keyboard Undo */
	KeyCut = 0x7b, /* Keyboard Cut */
	KeyCopy = 0x7c, /* Keyboard Copy */
	KeyPaste = 0x7d, /* Keyboard Paste */
	KeyFind = 0x7e, /* Keyboard Find */
	KeyMute = 0x7f, /* Keyboard Mute */
	KeyVolumeUp = 0x80, /* Keyboard Volume Up */
	KeyVolumeDown = 0x81, /* Keyboard Volume Down */
	KeyCapsLockLocking = 0x82, /* Keyboard Locking Caps Lock */
	KeyNumLockLocking = 0x83, /* Keyboard Locking Num Lock */
	KeyScrollLockLocking = 0x84, /* Keyboard Locking Scroll Lock */
	KeyBRComma = 0x85, /* Keypad Brazilian Comma */
	KeyBREqual = 0x86, /* Keyboard Brazilian Equal Sign */
	KeyRo = 0x87, /* Keyboard International1 */
	KeyKatakanaHiragana = 0x88, /* Keyboard International2 */
	KeyYen = 0x89, /* Keyboard International3 */
	KeyHenken = 0x8a, /* Keyboard International4 */
	KeyMuhenkan = 0x8b, /* Keyboard International5 */
	KeyJPComma = 0x8c, /* Keyboard International6 */

	// 0x8d  Keyboard International7
	// 0x8e  Keyboard International8
	// 0x8f  Keyboard International9

	KeyHanguel = 0x90, /* Keyboard LANG1 */
	KeyHanja = 0x91, /* Keyboard LANG2 */
	KeyKatakana = 0x92, /* Keyboard LANG3 */
	KeyHiragana = 0x93, /* Keyboard LANG4 */
	KeyZenkakuHankaku = 0x94, /* Keyboard LANG5 */

	// 0x95  Keyboard LANG6
	// 0x96  Keyboard LANG7
	// 0x97  Keyboard LANG8
	// 0x98  Keyboard LANG9
	// 0x99  Keyboard Alternate Erase
	// 0x9a  Keyboard SysReq/Attention
	// 0x9b  Keyboard Cancel
	// 0x9c  Keyboard Clear
	// 0x9d  Keyboard Prior
	// 0x9e  Keyboard Return
	// 0x9f  Keyboard Separator
	// 0xa0  Keyboard Out
	// 0xa1  Keyboard Oper
	// 0xa2  Keyboard Clear/Again
	// 0xa3  Keyboard CrSel/Props
	// 0xa4  Keyboard ExSel

	// 0xb0  Keypad 00
	// 0xb1  Keypad 000
	// 0xb2  Thousands Separator
	// 0xb3  Decimal Separator
	// 0xb4  Currency Unit
	// 0xb5  Currency Sub-unit

	KeyNPLeftParentheses = 0xb6, /* Keypad ( */
	KeyNPRightParentheses = 0xb7, /* Keypad ) */

	// 0xb8  Keypad {
	// 0xb9  Keypad }
	// 0xba  Keypad Tab
	// 0xbb  Keypad Backspace
	// 0xbc  Keypad A
	// 0xbd  Keypad B
	// 0xbe  Keypad C
	// 0xbf  Keypad D
	// 0xc0  Keypad E
	// 0xc1  Keypad F
	// 0xc2  Keypad XOR
	// 0xc3  Keypad ^
	// 0xc4  Keypad %
	// 0xc5  Keypad <
	// 0xc6  Keypad >
	// 0xc7  Keypad &
	// 0xc8  Keypad &&
	// 0xc9  Keypad |
	// 0xca  Keypad ||
	// 0xcb  Keypad :
	// 0xcc  Keypad #
	// 0xcd  Keypad Space
	// 0xce  Keypad @
	// 0xcf  Keypad !
	// 0xd0  Keypad Memory Store
	// 0xd1  Keypad Memory Recall
	// 0xd2  Keypad Memory Clear
	// 0xd3  Keypad Memory Add
	// 0xd4  Keypad Memory Subtract
	// 0xd5  Keypad Memory Multiply
	// 0xd6  Keypad Memory Divide
	// 0xd7  Keypad +/-
	// 0xd8  Keypad Clear
	// 0xd9  Keypad Clear Entry
	// 0xda  Keypad Binary
	// 0xdb  Keypad Octal
	// 0xdc  Keypad Decimal
	// 0xdd  Keypad Hexadecimal

	KeyLeftControl = 0xe0, /* Keyboard Left Control */
	KeyLeftShift = 0xe1, /* Keyboard Left Shift */
	KeyLeftAlt = 0xe2, /* Keyboard Left Alt */
	KeyLeftMeta = 0xe3, /* Keyboard Left GUI */
	KeyRightControl = 0xe4, /* Keyboard Right Control */
	KeyRightShift = 0xe5, /* Keyboard Right Shift */
	KeyRightAlt = 0xe6, /* Keyboard Right Alt */
	KeyRightMeta = 0xe7, /* Keyboard Right GUI */

	MediaPlayPause = 0xe8, /* Keyboard Media Play/Plause */
	MediaStopCD = 0xe9, /* Keyboard Media StopCD */
	MediaPrevious = 0xea, /* Keyboard Media Previous */
	MediaNext = 0xeb, /* Keyboard Media Next */
	MediaEjectCD = 0xec, /* Keyboard Media Eject CD */
	MediaVolumeUp = 0xed, /* Keyboard Media Volume UP */
	MediaVolumeDown = 0xee, /* Keyboard Media Volume Down */
	MediaMute = 0xef, /* Keyboard Media Mute */
	MediaWWW = 0xf0, /* Keyboard Media WWW Web Browser */
	MediaBack = 0xf1, /* Keyboard Media Back */
	MediaForward = 0xf2, /* Keyboard Media Forward */
	MediaStop = 0xf3, /* Keyboard Media Stop */
	MediaFind = 0xf4, /* Keyboard Media Find */
	MediaScrollUp = 0xf5, /* Keyboard Media Scroll Up */
	MediaScrollDown = 0xf6, /* Keyboard Media Scroll Down */
	MediaEdit = 0xf7, /* Keyboard Media Edit */
	MediaSleep = 0xf8, /* Keyboard Media Sleep */
	MediaCoffee = 0xf9, /* Keyboard Media Coffee */
	MediaRefresh = 0xfa, /* Keyboard Media Refresh */
	MediaCalculator = 0xfb, /* Keyboard Media Calc */
};

static const u8 map[16][6] = {
	{
		KeyESC,
		KeyGrave,
		KeyTab,
		KeyCapslock,
		KeyLeftShift,
		KeyLeftControl,
	},
	{
		KeyF1,
		Key1,
		None,
		None,
		KeyLeftShift,
		KeyLeftMeta,
	},
	{
		KeyF2,
		Key2,
		KeyQ,
		KeyA,
		KeyZ,
		KeyLeftAlt,
	},
	{
		KeyF3,
		Key3,
		KeyW,
		KeyS,
		KeyX,
		None,
	},
	{
		KeyF4,
		Key4,
		KeyE,
		KeyD,
		KeyC,
		None,
	},
	{
		KeyF5,
		Key5,
		KeyR,
		KeyF,
		KeyV,
		KeySpace,
	},
	{
		KeyF6,
		Key6,
		KeyT,
		KeyG,
		KeyB,
		None,
	},
	{
		KeyF7,
		Key7,
		KeyY,
		KeyH,
		KeyN,
		None,
	},
	{
		KeyF8,
		Key8,
		KeyU,
		KeyJ,
		KeyM,
		None,
	},
	{
		KeyF9,
		Key9,
		KeyI,
		KeyK,
		KeyComma,
		None,
	},
	{
		KeyF10,
		Key0,
		KeyO,
		KeyL,
		KeyPeriod,
		KeyRightAlt,
	},
	{
		KeyF11,
		KeyMinus,
		KeyP,
		KeySemicolon,
		KeyForwardSlash,
		KeyRightMeta,
	},
	{
		KeyF12,
		KeyEqual,
		KeyLeftBrace,
		KeyApostrophe,
		None,
		KeyRightControl,
	},
	{
		KeyInsert,
		None,
		KeyRightBrace,
		KeyBackSlash,
		KeyRightShift,
		KeyLeft,
	},
	{
		KeyDelete,
		KeyBackspace,
		KeyEnter,
		None,
		KeyUp,
		KeyDown,
	},
	{
		KeyPrintScreen,
		KeyHome,
		KeyEnd,
		KeyPageUp,
		KeyPageDown,
		KeyRight,
	},
};

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

	struct gpio_pin_t row_ios[] = {
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

	struct gpio_pin_t col_ios[] = {
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

	for (size_t i = 0; i < 21; i++) gpio_setup_pin(&gpio_config, row_ios[i], GPIO_MODE_PUSHPULL, 0);

	for (size_t i = 0; i < 12; i++) gpio_setup_pin(&gpio_config, col_ios[i], GPIO_MODE_INPUTPULLFILTER, 0);

	gpio_apply_config(gpio_config);

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

	struct keyboard_report report;
	u8 key_arr[16];

	for (;;) {
		tud_task();

		key_arr[0] = 0;

		for (size_t row = 0; row < 16; row++) {
			gpio_set(row_ios[row], 1);
			for (size_t col = 0; col < 6; col++) {
				if (gpio_get(col_ios[col])) {
					key_arr[0] = map[row][col];
				}
			}
			gpio_set(row_ios[row], 0);
		}

		if (tud_hid_ready()) {
			/* fill report */
			memset(&report, 0, sizeof(report));
			report.id = KEYBOARD_REPORT_ID;
			report.keys[0] = key_arr[0];

			tud_hid_report(0, &report, sizeof(report));
		}
	}
}
