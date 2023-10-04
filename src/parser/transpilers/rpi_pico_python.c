#include "../scanner.h"
#include "../token_type.h"
#include "../../helpers/formatter.h"
#include <stdio.h>
#include <string.h>

bool has_type = false;

char* type(Token* token) {
  char* resulting_code;
  if ( !has_type ) {
    has_type = true;
    resulting_code = "def type(text):\nlayout.write(text)\n";
  }

  strcat(resulting_code, formatter("type(%s)\n", token->literal));

  return resulting_code;
}

char* rpi_pico_python_transpiler(struct TokenArray* token_array) {
  Token* current_token;

  char* transpiled_code = "import usb_hid\nfrom adafruit_hid.keyboard import Keyboard\nfrom adafruit_hid.keyboard_layout_us import KeyboardLayoutUS as KeyboardLayout\nfrom adafruit_hid.keycode import Keycode\nkbd=Keyboard(usb_hid.devices)\nlayout=KeyboardLayout(kbd)\n";

  for ( int i = 0; i < token_array->size; i++ ) {
    current_token = token_array->array[i];
    
    char* resulting_code;

    switch (current_token->type) {
      case TYPE:
        i++;
        resulting_code = type(token_array->array[i]);
        break;

      default:
        break;
    }

    strcat(transpiled_code, resulting_code);
  }

  return "";
}
