#ifndef pikey_chunk_h
#define pikey_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
	OP_CONSTANT,
	OP_NULL,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_GET_LOCAL,
	OP_GET_GLOBAL,
	OP_DEFINE_GLOBAL,
	OP_SET_LOCAL,
	OP_ADD_SET_LOCAL,
	OP_SUB_SET_LOCAL,
	OP_SET_GLOBAL,
	OP_ADD_SET_GLOBAL,
	OP_SUB_SET_GLOBAL,
	OP_GET_UPVALUE,
	OP_SET_UPVALUE,
	OP_ADD_SET_UPVALUE,
	OP_SUB_SET_UPVALUE,
	OP_EQUAL,
	OP_GREATER,
	OP_LESSER,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_MODULO,
	OP_POW,
	OP_ANDB,
	OP_ORB,
	OP_XORB,
	OP_SHIFTR,
	OP_SHIFTL,
	OP_NOT,
	OP_NEGATE,
	OP_TYPE,
	OP_JUMP,
	OP_JUMP_IF_FALSE,
	OP_LOOP,
	OP_CALL,
	OP_CLOSURE,
	OP_CLOSE_UPVALUE,
	OP_RETURN,
	OP_WAIT,
	OP_CREATE_LIST,
	OP_SUBSCRIPT,
	OP_SET_SUBSCRIPT,
} OpCode;

typedef struct {
	int count;
	int capacity;
	uint8_t* code;
	int* lines;
	ValueArray constants;
} Chunk;

void init_chunk(Chunk* chunk);

void free_chunk(Chunk* chunk);

void write_chunk(Chunk* chunk, uint8_t byte, int line);

int add_constant(Chunk* chunk, Value value);

#endif // !pikey_chunk_h
