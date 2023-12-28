#include <stdint.h>

#include "tusb.h"
#include "pico/unique_id.h"
#include "usb_descriptors.h"

#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << n)
#define USB_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

#define USB_VID 0xcafe
#define USB_BCD 0x0200

tusb_desc_device_t const desc_device = {
	.bLength            = sizeof(tusb_desc_device_t),
	.bDescriptorType    = TUSB_DESC_DEVICE,
	.bcdUSB             = USB_BCD,
	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,
	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.bcdDevice          = 0x0100,

	.iManufacturer      = 0x01,
	.iProduct           = 0x02,
	.iSerialNumber      = 0x03,

	.bNumConfigurations = 0x01
};

uint8_t const* tud_descriptor_device_cb(void) {
	return (uint8_t const*)&desc_device;
}

uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(REPORT_ID_KEYBOARD         )),
  TUD_HID_REPORT_DESC_MOUSE   ( HID_REPORT_ID(REPORT_ID_MOUSE            )),
  TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL )),
  TUD_HID_REPORT_DESC_GAMEPAD ( HID_REPORT_ID(REPORT_ID_GAMEPAD          ))
};

uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
	(void)instance;
	return desc_hid_report;
}

enum {
	ITF_NUM_HID,
	ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID 0x81

uint8_t const desc_configuration[] = {
	TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
	TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
};

#if TUD_OPT_HIGH_SPEED

uint8_t desc_other_speed_config[CONFIG_TOTAL_LEN];

tusb_desc_device_qualifier_t const desc_device_qualifier = {
	.bLength            = sizeof(tusb_desc_device_qualifier_t),
	.bDescriptorType    = TUSB_DESC_DEVICE_QUALIFIER,
	.bcdUSB             = USB_BCD,

	.bDeviceClass       = 0x00,
	.bDeviceSubClass    = 0x00,
	.bDeviceProtocol    = 0x00,

	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
	.bNumConfigurations = 0x01,
	.bReserved          = 0x00
};

uint8_t tud_descriptor_device_qualifier_cb(void) {
	return (uint8_t const*)&desc_device_qualifier;
}
uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t index) {
	(void)index;

	memcpy(desc_other_speed_config, desc_configuration, CONFIG_TOTAL_LEN);
	desc_other_speed_config[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

	return desc_other_speed_config;
}

#endif // TUD_OPT_HIGH_SPEED

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
	(void)index;
	return desc_configuration;
}

char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

char const* string_desc_arr[] = {
	(const char[]) { 0x09, 0x04 }, // Supported language - English  TODO: custom languages ?
	"Pikey",                       // Manufacturer                  TODO: Change naming
	"PicoKey",                     // Product                       TODO: Change naming
	serial,                        // Serial, uses the flash ID     TODO: Change to custom eventually
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
	(void)langid;

	uint8_t char_count;

	if ( index == 0 ) {
		memcpy(&_desc_str[1], string_desc_arr[0], 2);
		char_count = 1;
	} else {
		if ( index == 3 ) {
			pico_get_unique_board_id_string(serial, sizeof(serial));
		}

		if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) ) return NULL;

		const char* str = string_desc_arr[index];

		char_count = strlen(str);
		if ( char_count > 31 ) {
			char_count = 31;
		}

		for ( int i=0; i < char_count; i++ ) {
			_desc_str[i + 1] = str[i];
		}
	}

	_desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * char_count + 2);

	return _desc_str;
}
