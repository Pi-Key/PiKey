#include <stdio.h>

void resetc() {
  printf("\033[0m");
}

void redc() {
  printf("\033[1;31m");
}

void yellowc() {
  printf("\033[1;33m");
}

void greenc() {
  printf("\003[1;32m");
}
