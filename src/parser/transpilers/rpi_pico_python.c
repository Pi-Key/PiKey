#include "../scanner.h"
#include "../token_type.h"
#include "../../helpers/formatter.h"
#include "../../helpers/pk_errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool has_type = false;
bool has_press = false;
bool has_release = false;
bool has_wait = false;

struct NumAndCode {
  int i;
  char* resulting_code;
};

char* type(Token* token) {
  char* resulting_code;
  if ( !has_type ) {
    has_type = true;
    resulting_code = "def type(text):\n    layout.write(text)\n";
  }

  if ( token->type != STRING && token->type != IDENTIFIER ) {
    pk_error(formatter("Type function on line %d was given an input other than a string or a variable.", token->line));
    exit(1);
  }

  strcat(resulting_code, formatter("type(%s)\n", token->lexeme));

  return resulting_code;
}

struct NumAndCode press(struct TokenArray* token_array, int current, bool keep) {
  struct NumAndCode result;

  if ( !has_press ) {
    has_press = true;
    result.resulting_code = "def press(keep, keys):\n    for key in keys:\n        command_keycode = duckyCommands.get(key, None)\n        if command_keycode is not None:\n            kbd.press(command_keycode)\n        elif hasattr(Keycode, key):\n            kdb.press(getattr(Keycode, key))\n    if not keep:\n        kdb.release_all()\n";
  }

  if ( keep ) {
    strcat(result.resulting_code, "press(True, [");
  } else {
    strcat(result.resulting_code, "press(False, [");
  }

  
  int i = current;
  while (token_array->array[i]->line == token_array->array[current]->line) {
    strcat(result.resulting_code, formatter("\"%s\"", token_array->array[i]->lexeme));
    i++;
  }

  strcat(result.resulting_code, "])\n");

  result.i = i - 1;

  return result;
}

struct NumAndCode release(struct TokenArray* token_array, int current) {
  struct NumAndCode result;

  if ( !has_release ) {
    has_release = true;
    result.resulting_code = "";
  }

  strcat(result.resulting_code, "kbd.release(");

  int i = current + 1;
  while (token_array->array[i]->line == token_array->array[current]->line) {
    strcat(result.resulting_code, formatter("Keycode.%s", token_array->array[i]->lexeme));
    i++;
  }

  strcat(result.resulting_code, ")\n");

  result.i = i - 1;

  return result;
}

struct NumAndCode let(struct TokenArray* token_array, int current) {
  struct NumAndCode result;

  if ( token_array->array[current + 1]->type != IDENTIFIER || token_array->array[current + 2]->type != EQUAL  ) {
    pk_error(formatter("[%d] There was an error with the variable assignment.", token_array->array[current]->line));
    exit(1);
  }

  result.resulting_code = formatter("%s = ", token_array->array[current + 1]->lexeme);

  int i = current + 3;
  while (token_array->array[i]->line == token_array->array[current]->line) {
    strcat(result.resulting_code, token_array->array[i]->lexeme);
    i++;
  }

  strcat(result.resulting_code, "\n");

  result.i = i - 1;

  return result;
}

char* wait(Token* token) {
  char* resulting_code;

  if ( !has_wait ) {
    has_wait = true;
    resulting_code = "import time\n";
  }

  if ( token->type != NUMBER ) {
    pk_error(formatter("[%d] The wait time amount must be a number.", token->line));
    exit(1);
  }

  strcat(resulting_code, formatter("time.sleep(int(%s)/1000)\n", token->literal));

  return resulting_code;
}

char* led(Token* token) {
  char* resulting_code;

  resulting_code = "led.value = ";

  if ( token->type == TRUE ) {
    strcat(resulting_code, "True\n");
  } else if ( token->type == FALSE ) {
    strcat(resulting_code, "False\n");
  } else {
    pk_error(formatter("[%d] 'led' received a value other than true or false", token->line));
    exit(1);
  }

  return resulting_code;
}

char* mode(Token* token) {
  char* resulting_code;

  if ( token->type != IDENTIFIER ) {
    pk_error(formatter("[%d] 'mode' only accepts one of four inputs (key, mass, both, none).", token->line));
    exit(1);
  }

  char* desired_mode = token->lexeme;

  if ( strcmp(desired_mode, "key") ) {
    resulting_code = "storage.disable_usb_drive()\nusb_hid.enable()\n";
  } else if ( strcmp(desired_mode, "mass") ) {
    resulting_code = "usb_hid.disable()\nstorage.enable_usb_drive()\n";
  } else if ( strcmp(desired_mode, "both") ) {
    resulting_code = "usb_hid.enable()\nstorage.enable_usb_drive()\n";
  } else if ( strcmp(desired_mode, "none") ) {
    resulting_code = "usb_hid.disable()\nstorage.disable_usb_drive()\n";
  } else {
    pk_error(formatter("[%d] 'mode' only accepts one of four inputs (key, mass, both, none).", token->line));
    exit(1);
  }

  return resulting_code;
}

char* rpi_pico_python_transpiler(struct TokenArray* token_array) {
  Token* current_token;

  char* transpiled_code = "import board\nfrom board import\nimport usb_hid\nfrom adafruit_hid.keyboard import Keyboard\nfrom adafruit_hid.keyboard_layout_us import KeyboardLayoutUS as KeyboardLayout\nfrom adafruit_hid.keycode import Keycode\n\nkbd=Keyboard(usb_hid.devices)\nlayout=KeyboardLayout(kbd)\n\ndef define_ducky_commands():\n    return {\n        'WINDOWS': Keycode.WINDOWS, 'GUI': Keycode.GUI,\n        'APP': Keycode.APPLICATION, 'MENU': Keycode.APPLICATION, 'SHIFT': Keycode.SHIFT,\n        'ALT': Keycode.ALT, 'CONTROL': Keycode.CONTROL, 'CTRL': Keycode.CONTROL,\n        'DOWNARROW': Keycode.DOWN_ARROW, 'DOWN': Keycode.DOWN_ARROW, 'LEFTARROW': Keycode.LEFT_ARROW,\n        'LEFT': Keycode.LEFT_ARROW, 'RIGHTARROW': Keycode.RIGHT_ARROW, 'RIGHT': Keycode.RIGHT_ARROW,\n        'UPARROW': Keycode.UP_ARROW, 'UP': Keycode.UP_ARROW, 'BREAK': Keycode.PAUSE,\n        'PAUSE': Keycode.PAUSE, 'CAPSLOCK': Keycode.CAPS_LOCK, 'DELETE': Keycode.DELETE,\n        'END': Keycode.END, 'ESC': Keycode.ESCAPE, 'ESCAPE': Keycode.ESCAPE, 'HOME': Keycode.HOME,\n        'INSERT': Keycode.INSERT, 'NUMLOCK': Keycode.KEYPAD_NUMLOCK, 'PAGEUP': Keycode.PAGE_UP,\n        'PAGEDOWN': Keycode.PAGE_DOWN, 'PRINTSCREEN': Keycode.PRINT_SCREEN, 'ENTER': Keycode.ENTER,\n        'SCROLLLOCK': Keycode.SCROLL_LOCK, 'SPACE': Keycode.SPACE, 'TAB': Keycode.TAB,\n        'BACKSPACE': Keycode.BACKSPACE,\n        'A': Keycode.A, 'B': Keycode.B, 'C': Keycode.C, 'D': Keycode.D, 'E': Keycode.E,\n        'F': Keycode.F, 'G': Keycode.G, 'H': Keycode.H, 'I': Keycode.I, 'J': Keycode.J,\n        'K': Keycode.K, 'L': Keycode.L, 'M': Keycode.M, 'N': Keycode.N, 'O': Keycode.O,\n        'P': Keycode.P, 'Q': Keycode.Q, 'R': Keycode.R, 'S': Keycode.S, 'T': Keycode.T,\n        'U': Keycode.U, 'V': Keycode.V, 'W': Keycode.W, 'X': Keycode.X, 'Y': Keycode.Y,\n        'Z': Keycode.Z, 'F1': Keycode.F1, 'F2': Keycode.F2, 'F3': Keycode.F3,\n        'F4': Keycode.F4, 'F5': Keycode.F5, 'F6': Keycode.F6, 'F7': Keycode.F7,\n        'F8': Keycode.F8, 'F9': Keycode.F9, 'F10': Keycode.F10, 'F11': Keycode.F11,\n        'F12': Keycode.F12,\n    }\nduckyCommands = define_ducky_commands()\n\n";

  for ( int i = 0; i < token_array->size; i++ ) {
    current_token = token_array->array[i];
    
    char* resulting_code;

    switch (current_token->type) {
      case TYPE:
        i++;
        resulting_code = type(token_array->array[i]->literal);
        break;
      case TYPELN:
        i++;
        strcat(token_array->array[i]->literal, "\n");
        resulting_code = type(token_array->array[i]->literal);
        break;
      case PRESS:
        {
          struct NumAndCode new_i_and_code;
          new_i_and_code = press(token_array, i, false);
          resulting_code = new_i_and_code.resulting_code;
          i = new_i_and_code.i;
          break;
        }
      case PRESS_DOWN:
        {
          struct NumAndCode new_i_and_code;
          new_i_and_code = press(token_array, i, true);
          resulting_code = new_i_and_code.resulting_code;
          i = new_i_and_code.i;
          break;
        }
      case RELEASE:
        {
          struct NumAndCode new_i_and_code;
          new_i_and_code = release(token_array, i);
          resulting_code = new_i_and_code.resulting_code;
          i = new_i_and_code.i;
          break;
        }
      case LET:
        {
          struct NumAndCode new_i_and_code;
          new_i_and_code = let(token_array, i);
          resulting_code = new_i_and_code.resulting_code;
          i = new_i_and_code.i;
          break;
        }
      case WAIT:
        i++;
        resulting_code = wait(token_array->array[i]);
        break;
      case LED:
        i++;
        resulting_code = led(token_array->array[i]);
        break;
      case MODE:
        i++;
        resulting_code = mode(token_array->array[i]);
        break;

      default:
        break;
    }

    strcat(transpiled_code, resulting_code);
  }

  return "";
}
