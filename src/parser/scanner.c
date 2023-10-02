#include "../helpers/pk_errors.h"
#include "../helpers/file_utils.h"
#include "../helpers/substring.h"
#include "./token.h"
#include "./token_type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct TokenArray {
  Token** array;
  size_t size;
};

typedef struct {
  struct FileContent content;
  char* source;
  int current;
  int start;
  int line;
  struct TokenArray* tokens;
} Scanner;

char* get_line_from_mem(struct FileContent content, int line) {
  int start_pos = 0;
  int end_pos = 0;
  int current_line = 1;

  for ( int i = 0; i < content.size; i++ ) {
    if ( content.bytes[i] == '\n' ) {
      current_line++;
      if ( current_line == line ) {
        end_pos = i;
        break;
      }
      start_pos = i + 1;
    }
  }

  if ( current_line < line ) {
    return NULL;
  }

  int line_length = end_pos - start_pos;
  char* line_content = malloc(line_length + 1);
  if ( line_content == NULL ) {
    return NULL;
  }

  memcpy(line_content, content.bytes + start_pos, line_length);
  line_content[line_length] = '\0';

  return line_content;
}

void line_error(int line, char* message, struct FileContent content) {
  char* line_content = get_line_from_mem(content, line);

  if ( line_content == NULL ) {
    char error_msg[100];
    sprintf(error_msg, "%s on line %d", message, line);
    pk_error(error_msg);
  }

  printf("Error: %s\n", message);
  printf("%d | %s\n", line, line_content);
}

struct TokenArray* create_token_array() {
  struct TokenArray* token_array = malloc(sizeof(struct TokenArray));
  token_array->array = NULL;
  token_array->size = 0;
  return token_array;
}

void free_token_array(struct TokenArray* token_array) {
  free(token_array->array);
  free(token_array);
}

void init_scanner(Scanner* scanner, struct FileContent content) {
  scanner->content = content;
  scanner->source = content.bytes;
  scanner->current = 0;
  scanner->start = 0;
  scanner->line = 1;
  scanner->tokens = create_token_array();
}

char advance(Scanner* scanner) {
  return scanner->source[scanner->current++];
}

void add_token_lit(Scanner* scanner, enum TokenType type, void* literal) {
  char* text = substring(scanner->source, scanner->start, scanner->current);

  scanner->tokens->array = realloc(scanner->tokens->array, (scanner->tokens->size + 1) * sizeof(struct Token*));

  (scanner->tokens->array)[scanner->tokens->size] = create_token((TokenType)type, text, literal, scanner->line);
  scanner->tokens->size++;
}

void add_token(Scanner* scanner, enum TokenType type) {
  add_token_lit(scanner, type, NULL); 
}

bool is_at_end(Scanner* scanner) {
  return scanner->current >= strlen(scanner->source);
}

bool match(Scanner* scanner, char expected) {
  if ( is_at_end(scanner) ) return false;

  if ( scanner->source[scanner->current] != expected ) return false;

  scanner->current++;
  return true;
}

char peek(Scanner* scanner) {
  if ( is_at_end(scanner) ) return '\0';

  return scanner->source[scanner->current];
}

void string(Scanner* scanner) {
  while ( peek(scanner) != '"' && !is_at_end(scanner) ) {
    if ( peek(scanner) == '\n' ) scanner->line++;
    advance(scanner);
  }

  if ( is_at_end(scanner) ) {
    line_error(scanner->line, "Unterminated string", scanner->content);
  }

  advance(scanner);
  
  char* value = substring(scanner->source, scanner->start+1, scanner->current-1);

  add_token_lit(scanner, STRING, value);
}

bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

char peek_next(Scanner* scanner) {
  if ( scanner->current + 1 >= strlen(scanner->source) ) return '\0';
  return scanner->source[scanner->current + 1];
}

void number(Scanner* scanner) {
  while ( is_digit(peek(scanner)) ) advance(scanner);

  if ( peek(scanner) == '.' && is_digit(peek_next(scanner))) {
    advance(scanner);

    while ( is_digit(peek(scanner)) ) advance(scanner);
  }

  char* ptr;

  add_token_lit(scanner, NUMBER, substring(scanner->source, scanner->start, scanner->current));
}

bool is_alpha(char c) {
  return  (c >= 'a' && c <= 'z') ||
          (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool is_alpha_num(char c) {
  return is_alpha(c) || is_digit(c);
}

enum TokenType get_keyword_type(const char* keyword) {
  const char* keywords[] = {
    "and", "or", "xor", "if", "elif", "for", "while", "def", "true", "false",
    "let", "type", "typeln", "press", "press_down", "release", "wait", "led",
    "mode", "vendor_id", "product_id", "manufacturer", "product", "serial_num",
  };

  enum TokenType token_types[] = {
    AND, OR, XOR, IF, ELIF, FOR, WHILE, DEF, TRUE, FALSE,
    LET, TYPE, TYPELN, PRESS, PRESS_DOWN, RELEASE, WAIT, LED,
    MODE, VENDOR_ID, PRODUCT_ID, MANUFACTURER, PRODUCT, SERIAL_NUM,
  };

  for ( int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++ ) {
    if ( strcmp(keyword, keywords[i]) == 0 ) {
      return token_types[i];
    }
  }

  return -1;
}

void identifier(Scanner* scanner) {
  while ( is_alpha_num(peek(scanner)) ) advance(scanner);

  char* text = substring(scanner->source, scanner->start, scanner->current);
  enum TokenType type = get_keyword_type(text);

  if ( type == -1 ) type = IDENTIFIER;

  add_token(scanner, type);
}

void scan_token(Scanner* scanner) {
  char c = advance(scanner);

  switch (c) {
    case '(': add_token(scanner, LEFT_PARENTHESE); break;
    case ')': add_token(scanner, RIGHT_PARENTHESE); break;
    case '{': add_token(scanner, LEFT_BRACE); break;
    case '}': add_token(scanner, RIGHT_BRACE); break;
    case ',': add_token(scanner, COMMA); break;
    case '.': add_token(scanner, DOT); break;
    case '-': add_token(scanner, match(scanner, '=') ? MINUS_EQUAL : match(scanner, '>') ? ARROW : MINUS); break;
    case '+': add_token(scanner, match(scanner, '=') ? PLUS_EQUAL : PLUS); break;
    case '*': add_token(scanner, match(scanner, '*') ? DOUBLE_STAR : STAR); break;
    case '&': add_token(scanner, AMPERSAND); break;
    case '|': add_token(scanner, PIPE); break;
    case '^': add_token(scanner, CARET); break;
    case '%': add_token(scanner, PERCENT); break;

    case '!': add_token(scanner, match(scanner, '=') ? BANG_EQUAL : BANG); break;
    case '=': add_token(scanner, match(scanner, '=') ? EQUAL_EQUAL : match(scanner, '>') ? THICK_ARROW : EQUAL); break;
    case '<': add_token(scanner, match(scanner, '=') ? LESS_EQUAL : LESS_THAN); break;
    case '>': add_token(scanner, match(scanner, '=') ? GREATER_EQUAL : GREATER_THAN); break;

    case '/':
      if ( match(scanner, '/') ) {
        while ( peek(scanner) != '\n' && !is_at_end(scanner) ) advance(scanner);
      } else if ( match(scanner, '*') ) {
        char previous_char;
        while (!is_at_end(scanner) ) {
          if ( previous_char == '*' && match(scanner, '/') ) {
            break;
          }
          previous_char = peek(scanner);

          advance(scanner);
        }
      } else {
        add_token(scanner, SLASH);
      }

    case ' ':
    case '\r':
    case '\t':
      break;

    case '\n':
      scanner->line++;
      break;

    case '"': string(scanner); break;

    default:
      if ( is_digit(c) ) {
        number(scanner);
      } else if ( is_alpha(c) ) {
        identifier(scanner);
      } else {
        line_error(scanner->line, strcat("Unexpected character", (char*)&c), scanner->content);
      }
      break;
  }
}

void scan_tokens(Scanner* scanner) {
  while ( !is_at_end(scanner) ) {
    scanner->start = scanner->current;
    scan_token(scanner);
  }

  scanner->tokens->array = realloc(scanner->tokens->array, (scanner->tokens->size + 1) * sizeof(struct Token*));
  (scanner->tokens->array)[scanner->tokens->size] = create_token(EOF, "", NULL, scanner->line);
  scanner->tokens->size++;
}

struct TokenArray* scan_file(char* file_path) {
  struct FileContent content = readFile(file_path);

  Scanner scanner;
  init_scanner(&scanner, content);

  scan_tokens(&scanner);

  for ( int i = 0; i < scanner.tokens->size; i++ ) {
    printf("%u %s %d\n", scanner.tokens->array[i]->type, scanner.tokens->array[i]->lexeme, scanner.tokens->array[i]->line);
  }

  return scanner.tokens;
}
