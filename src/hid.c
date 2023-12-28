#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "pico/cyw43_arch.h"

#include "keys.h"
#include "usb_descriptors.h"

enum {
	BLINK_NOT_MOUNTED = 250,
	BLINK_MOUNTED     = 1000,
	BLINK_SUSPENDED   = 2500
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

void start_hid(void) {
	board_init();
	tusb_init();


	while ( 1 ) {
		tud_task();
	}
}

void tud_mount_cb(void) {
	blink_interval_ms = BLINK_MOUNTED;
}

void tud_unmount_cb(void) {
	blink_interval_ms = BLINK_NOT_MOUNTED;
}

void tud_suspend_cb(bool remote_wakeup_en) {
	(void)remote_wakeup_en;
	blink_interval_ms = BLINK_SUSPENDED;
}

void tud_resume_cb(void) {
	blink_interval_ms = BLINK_MOUNTED;
}

static void send_keyboard_hid_report(uint8_t* keycode) {
	if ( !tud_hid_ready() ) return;

	tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);

	sleep_ms(5);

	uint8_t nocode[6] = { 0 };
	nocode[0] = HID_KEY_NONE;

	tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, nocode);

	sleep_ms(5);
}

void send_key(char* input) {
	uint8_t keycode[6] = { 0 };
	keycode[0] = string_to_keycode(input);

	send_keyboard_hid_report(keycode);
}

void send_word(char* input) {
	for ( int i=0; i < strlen(input); i++ ) {
		uint8_t keycode[6] = { 0 };
		keycode[0] = string_to_keycode(&input[i]);

		send_keyboard_hid_report(keycode);
	}
}

static void send_hid_report(uint8_t report_id, uint32_t btn) {
	if ( !tud_hid_ready() ) return;
	
	switch ( report_id ) {
		case REPORT_ID_KEYBOARD: {
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
			uint8_t keycode[6] = { 0 };
			keycode[0] = HID_KEY_A;

			tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);

			sleep_ms(25);

			uint8_t nocode[6] = { 0 };
			nocode[0] = HID_KEY_NONE;
			tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, nocode);
			cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
			break;
		}
		default: break;
	}
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
	(void)instance;
	(void)len;

	uint8_t next_report_id = report[0] + 1;

	if ( next_report_id < REPORT_ID_COUNT ) {
		send_hid_report(next_report_id, false);
	}
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
	(void)instance;
	(void)report_id;
	(void)report_type;
	(void)buffer;
	(void)reqlen;

	return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
	(void)instance;
 
	if ( report_type == HID_REPORT_TYPE_OUTPUT ) {
		if ( report_id == REPORT_ID_KEYBOARD ) {
			if ( bufsize < 1 ) return;

			uint8_t const kbd_leds = buffer[0];

			if ( kbd_leds & KEYBOARD_LED_CAPSLOCK ) {
				blink_interval_ms = 0;
				board_led_write(true);
			} else {
				board_led_write(false);
				blink_interval_ms = BLINK_MOUNTED;
			}
		}
	}
}
