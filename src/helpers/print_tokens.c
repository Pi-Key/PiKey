#include "../parser/scanner.h"
#include <stdio.h>
#include "colors.h"

void print_tokens(struct TokenArray* tokens) {
  int prev_line = 1;

  for ( int i = 0; i < tokens->size; i++ ) {
    if ( tokens->array[i]->line > prev_line ) {
      printf("\n");
      prev_line = tokens->array[i]->line;
    }
    
    printf(GREEN "%u", tokens->array[i]->type);

    printf(RESET " # ");

    printf(YELLOW "%s", tokens->array[i]->lexeme);

    printf(RESET " # ");

    printf(RED "%d\n", tokens->array[i]->line);
    resetc();
  }
}
