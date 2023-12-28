#include "tusb.h"

#include "keys.h"

uint8_t string_to_keycode(char* string) {
	char firstc = string[0];

	uint8_t key_start = 0;

	switch ( firstc ) {
		case 'A':
			key_start = HID_KEY_A;
			break;
		case 'B':
			key_start = HID_KEY_B;
			break;
		case 'C':
			key_start = HID_KEY_C;
			break;
		case 'D':
			key_start = HID_KEY_D;
			break;
		case 'E':
			key_start = HID_KEY_E;
			break;
		case 'F':
			key_start = HID_KEY_F;
			break;
		case 'G':
			key_start = HID_KEY_G;
			break;
		case 'H':
			key_start = HID_KEY_H;
			break;
		case 'I':
			key_start = HID_KEY_I;
			break;
		case 'J':
			key_start = HID_KEY_J;
			break;
		case 'K':
			key_start = HID_KEY_K;
			break;
		case 'L':
			key_start = HID_KEY_L;
			break;
		case 'M':
			key_start = HID_KEY_M;
			break;
		case 'N':
			key_start = HID_KEY_N;
			break;
		case 'O':
			key_start = HID_KEY_O;
			break;
		case 'P':
			key_start = HID_KEY_P;
			break;
		case 'Q':
			key_start = HID_KEY_Q;
			break;
		case 'R':
			key_start = HID_KEY_R;
			break;
		case 'S':
			key_start = HID_KEY_S;
			break;
		case 'T':
			key_start = HID_KEY_T;
			break;
		case 'U':
			key_start = HID_KEY_U;
			break;
		case 'V':
			key_start = HID_KEY_V;
			break;
		case 'W':
			key_start = HID_KEY_W;
			break;
		case 'X':
			key_start = HID_KEY_X;
			break;
		case 'Y':
			key_start = HID_KEY_Y;
			break;
		case 'Z':
			key_start = HID_KEY_Z;
			break;

		default: break;
	}

	return key_start;
}
