#ifndef TOKEN
#define TOKEN

#include "token_type.h"

// typedef enum {
//   TOKEN_TYPE,
//   STRING_TYPE,
//   NUMBER_TYPE,
// } TokenType;

typedef struct {
  enum TokenType type;
  char* lexeme;
  void* literal;
  int line;
} Token;

Token* create_token(enum TokenType type, const char* lexeme, void* literal, int line);

void free_token(Token* token);

char* token_to_string(Token* token);

#endif
