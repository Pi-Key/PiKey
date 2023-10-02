#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* substring(char* string, int start, int end) {
  int length = end - start;

  char* substr = malloc(length);
  sprintf(substr, "%.*s", length, string+start);

  return substr;
}
