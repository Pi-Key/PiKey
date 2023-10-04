#include "../scanner.h"
#include "../token_type.h"
#include <string.h>

char* type(Token* token) {
  return "";
}

char* rpi_pico_python_transpiler(struct TokenArray* token_array) {
  Token* current_token;

  char* transpiled_code;

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
