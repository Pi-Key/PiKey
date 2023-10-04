#include "./scanner.h"
#include <string.h>
#include "./transpilers/rpi_pico_python.h"

void file_parser(char* file_path, char* destination) {
  struct TokenArray* token_array = scan_file(file_path);

  // if ( strcmp(destination, "rpi-pico-python") ) {
  //   rpi_pico_python_transpiler(token_array);
  // }
}
