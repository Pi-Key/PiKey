#include "../scanner.h"
#include "../token_type.h"

char* rpi_pico_python_transpiler(struct TokenArray* token_array) {
  Token* current_token;

  for ( int i = 0; i < token_array->size; i++ ) {
    current_token = token_array->array[i];
    
    switch (current_token->type) {
      case TYPE: break;

      default:
        break;
    }
  }

  return "";
}
