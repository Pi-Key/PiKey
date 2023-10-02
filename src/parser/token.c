#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token* create_token(TokenType type, const char* lexeme, void* literal, int line) {
  Token* token = (Token*)malloc(sizeof(Token));
  token->type = type;
  token->lexeme = strdup(lexeme);
  token->literal = literal;
  token->line = line;

  return token;
}

void free_token(Token* token) {
  free(token->lexeme);
  free(token);
}

char* token_to_string(Token* token) {
  char* result = (char*)malloc(100);

  sprintf(result, "%d %s ", token->type, token->lexeme);
  
  if ( token->type == STRING_TYPE ){
    sprintf(result, "\"%s\"", (char*)token->literal);
  } else if ( token->type == NUMBER_TYPE ) {
    sprintf(result, "%s", (char*)token->literal);
  }

  return result;
}
