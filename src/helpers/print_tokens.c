#include "../parser/scanner.h"
#include <stdio.h>

void print_tokens(struct TokenArray* tokens) {
  int prev_line = 1;
  for ( int i = 0; i < tokens->size; i++ ) {
    if ( tokens->array[i]->line > prev_line ) {
      printf("\n");
      prev_line = tokens->array[i]->line;
    }
    printf("%s", tokens->array[i]->lexeme);
  }
}
