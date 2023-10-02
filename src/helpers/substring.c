#include <stdlib.h>
#include <string.h>

char* substring(char* string, int start, int end) {
  int length = end - start;

  char* substr = malloc(length);

  strncpy(substr, string+start, length);

  return substr;
}
