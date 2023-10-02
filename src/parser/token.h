#ifndef TOKEN
#define TOKEN

typedef enum {
  TOKEN_TYPE,
  STRING_TYPE,
  NUMBER_TYPE,
} TokenType;

typedef struct {
  TokenType type;
  char* lexeme;
  void* literal;
  int line;
} Token;

Token* create_token(TokenType type, const char* lexeme, void* literal, int line);

void free_token(Token* token);

char* token_to_string(Token* token);

#endif
