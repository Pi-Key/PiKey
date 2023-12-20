#include <stdbool.h>
#include <string.h>

#include "scanner.h"

typedef struct {
	const char* start;
	const char* current;
	int line;
} Scanner;

Scanner scanner;

void init_scanner(const char *source) {
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}

static bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		c == '_';
}

static bool is_digit(char c) {
	return c >='0' && c <= '9';
}

static bool is_at_end() {
	return *scanner.current == '\0';
}

static char advance() {
	scanner.current++;
	return scanner.current[-1];
}

static char peek() {
	return *scanner.current;
}

static char peek_next() {
	if ( is_at_end() ) return '\0';

	return scanner.current[1];
}

static bool match(char expected) {
	if ( is_at_end() ) return false;
	if ( *scanner.current != expected ) return false;
	scanner.current++;

	return true;
}

static Token make_token(TokenType type) {
	Token token;
	token.type = type;
	token.start = scanner.start;
	token.length = (int)(scanner.current - scanner.start);
	token.line = scanner.line;

	return token;
}

static Token error_token(const char* message) {
	Token token;
	token.type = TOKEN_ERROR;
	token.start = message;
	token.length = (int)strlen(message);
	token.line = scanner.line;

	return token;
}

static void skip_whitespace() {
	for (;;) {
		char c = peek();
		switch ( c ) {
			case ' ':
			case '\t':
			case '\r':
				advance();
				break;
			case '\n':
				scanner.line++;
				advance();
				break;
			case '/':
				if ( peek_next() == '/' ) {
					while ( peek() != '\n' && !is_at_end() ) advance();
				} else if ( peek_next() == '*' ) {
					advance();
					advance();
					while ( peek() != '*' && peek_next() != '/' && !is_at_end() ) advance();
					advance();
					advance();
				} else {
					return;
				}
				break;
			default:
				return;
		}
	}
}

static TokenType check_keyword(int start, int length, const char* rest, TokenType type) {
	if ( scanner.current - scanner.start == start + length &&
		memcmp(scanner.start + start, rest, length) == 0) {
		return type;
	}

	return TOKEN_IDENTIFIER;
}

static TokenType identifier_type() {
	switch ( scanner.start[0] ) {
		case 'a': return check_keyword(1, 2, "nd", TOKEN_AND);
		case 'd': return check_keyword(1, 2, "ef", TOKEN_FUNCTION);
		case 'e': return check_keyword(1, 3, "lse", TOKEN_ELSE);
		case 'f':
			if ( scanner.current - scanner.start > 1 ) {
				switch ( scanner.start[1] ) {
					case 'a': return check_keyword(2, 3, "lse", TOKEN_FALSE);
					case 'o': return check_keyword(2, 1, "r", TOKEN_FOR);
				}
			}
			break;
		case 'i': return check_keyword(1, 1, "f", TOKEN_IF);
		case 'l': return check_keyword(1, 2, "et", TOKEN_VAR);
		case 'n': return check_keyword(1, 3, "ull", TOKEN_NULL);
		case 'o': return check_keyword(1, 1, "r", TOKEN_OR);
		case 'r': return check_keyword(1, 5, "eturn", TOKEN_RETURN);
		case 't':
			if ( scanner.current - scanner.start > 1 ) {
				switch ( scanner.start[1] ) {
					case 'r': return check_keyword(2, 2, "ue", TOKEN_TRUE);
					case 'y': return check_keyword(2, 2, "pe", TOKEN_TYPE);
				}
			}
			break;
		case 'w': 
			if ( scanner.current - scanner.start > 1 ) {
				switch ( scanner.start[1] ) {
					case 'a': return check_keyword(2, 2, "it", TOKEN_WAIT);
					case 'h': return check_keyword(2, 3, "ile", TOKEN_WHILE);
				}
			}
			break;
	}

	return TOKEN_IDENTIFIER;
}

static Token identifier() {
	while ( is_alpha(peek()) || is_digit(peek()) ) advance();
	return make_token(identifier_type());
}

static Token number() {
	while ( is_digit(peek()) ) advance();

	if ( peek() == '.' && is_digit(peek_next()) ) {
		advance();

		while ( is_digit(peek()) ) advance();
	}

	return make_token(TOKEN_NUMBER);
}

static Token string(char quote_char) {
	while ( peek() != quote_char &&  !is_at_end() ) {
		if ( peek() == '\n' ) scanner.line++;

		advance();
	}

	if ( is_at_end() ) return error_token("Unterminated string.");

	advance();
	return make_token(TOKEN_STRING);
}

Token scan_token() {
	skip_whitespace();
	scanner.start = scanner.current;

	if ( is_at_end() ) return make_token(TOKEN_EOF);

	char c = advance();
	if ( is_alpha(c) ) return identifier();
	if ( is_digit(c) ) return number();

	switch ( c ) {
		case '(':  return make_token(TOKEN_LEFT_PAREN);
		case ')':  return make_token(TOKEN_RIGHT_PAREN);
		case '{':  return make_token(TOKEN_LEFT_BRACE);
		case '}':  return make_token(TOKEN_RIGHT_BRACE);
		case ';':  return make_token(TOKEN_SEMICOLON);
		case ',':  return make_token(TOKEN_COMMA);
		case '.':  return make_token(TOKEN_DOT);
		case '-':  return make_token(match('=') ? TOKEN_MINUS_EQUAL : TOKEN_MINUS);
		case '+':  return make_token(match('=') ? TOKEN_PLUS_EQUAL : TOKEN_PLUS);
		case '/':  return make_token(TOKEN_SLASH);
		case '*':  return make_token(match('*') ? TOKEN_STAR_STAR : TOKEN_STAR);
		case '!':  return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=':  return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<':  return make_token(match('=') ? TOKEN_LESSER_EQUAL : match('<') ? TOKEN_LESSER_LESSER : TOKEN_LESSER);
		case '>':  return make_token(match('=') ? TOKEN_GREATER_EQUAL : match('>') ? TOKEN_GREATER_GREATER : TOKEN_GREATER);
		case '^':  return make_token(TOKEN_CARET);
		case '|':  return make_token(TOKEN_PIPE);
		case '&':  return make_token(TOKEN_AMPERSAND);
		case '%':  return make_token(TOKEN_PERCENT);
		case '[':  return make_token(TOKEN_LEFT_BRACKET);
		case ']':  return make_token(TOKEN_RIGHT_BRACKET);
		case '"':  return string('"');
		case '\'': return string('\'');
	}

	return error_token("Unexpected character.");
}
