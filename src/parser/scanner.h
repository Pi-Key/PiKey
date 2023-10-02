#ifndef SCANNER
#define SCANNER

#include "./token.h"
#include <stddef.h>
#include <stdbool.h>

struct TokenArray {
  Token** array;
  size_t size;
};

typedef struct {
  struct FileContent* content;
  char* source;
  int current;
  int start;
  int line;
  struct TokenArray* tokens;
} Scanner;

struct TokenArray* scan_file(char* file_path);

#endif
