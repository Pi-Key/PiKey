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

void init_scanner(Scanner* scanner, char* source);

char advance(Scanner* scanner);

void add_token(TokenType type);

void scan_token(Scanner* scanner);

bool is_at_end(Scanner* scanner);

Token* scan_tokens(Scanner* scanner, Token* tokens, int* token_count);

char* get_line_from_mem(struct FileContent content, int line);

void line_error(int line, char* message, struct FileContent content);

int scan_file(char* file_path);

#endif
