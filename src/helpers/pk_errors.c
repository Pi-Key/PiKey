#include "./colors.h"
#include <stdio.h>

void pk_error(char* err_string) {
  redc();
  printf("[ERR]: %s\n", err_string);
  resetc();
}

void pk_warning(char* warn_string) {
  yellowc();
  printf("[ERR]: %s\n", warn_string);
  resetc();
}

void pk_info(char* info_string) {
  greenc();
  printf("[ERR]: %s\n", info_string);
  resetc();
}
