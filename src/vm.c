#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#include "chunk.h"
#include "compiler.h"
#include "object.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

VM vm;

static void runtime_error(const char* format, ...);

static Value clock_native(int arg_count, Value* args) {
	return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static Value lower_native(int arg_count, Value* args) {
	if ( arg_count != 1 ) {
		runtime_error("The lower function takes exactly one argument.");
		return INTERPRET_RUNTIME_ERROR;
	}

	if ( !IS_STRING(args[0]) ) {
		runtime_error("The lower function takes a string as an argument.");
		return INTERPRET_RUNTIME_ERROR;
	}

	char* string = AS_CSTRING(args[0]);

	for ( int i=0; string[i]; i++) {
		string[i] = tolower(string[i]);
	}
	return OBJ_VAL(copy_string(string, strlen(string)));
}

static Value upper_native(int arg_count, Value* args) {
	if ( arg_count != 1 ) {
		runtime_error("The upper function takes exactly one argument.");
		return INTERPRET_RUNTIME_ERROR;
	}

	if ( !IS_STRING(args[0]) ) {
		runtime_error("The upper function takes a string as an argument.");
		return INTERPRET_RUNTIME_ERROR;
	}

	char* string = AS_CSTRING(args[0]);

	for ( int i=0; string[i]; i++) {
		string[i] = toupper(string[i]);
	}
	return OBJ_VAL(copy_string(string, strlen(string)));
}

static double rand_num_gen(double min, double max) {
	return min + (rand() / (double)(RAND_MAX) * (max - min));
}

static Value rand_native(int arg_count, Value* args) {
	double min, max;
	if ( arg_count != 2 ) {
		min = 0;
		max = 1;
	} else {
		if ( !IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ) {
			runtime_error("The minimum and maximum for rand_num must be numbers.");
			return INTERPRET_RUNTIME_ERROR;
		}

		min = AS_NUMBER(args[0]);
		max = AS_NUMBER(args[1]);
	}

	double rand_num = rand_num_gen(min, max);
	return NUMBER_VAL(rand_num);
}

static Value rand_int_native(int arg_count, Value* args) {
	int min, max;
	if ( arg_count != 2 ) {
		min = 0;
		max = 1;
	} else {
		if ( !IS_NUMBER(args[0]) || !IS_NUMBER(args[1]) ) {
			runtime_error("The minimum and maximum for rand_num must be numbers.");
			return INTERPRET_RUNTIME_ERROR;
		}

		min = (int)AS_NUMBER(args[0]);
		max = (int)AS_NUMBER(args[1]);
	}

	int rand_num = (int)rand_num_gen(min, max + 1);
	return NUMBER_VAL(rand_num);
}

static Value rand_digit_native(int arg_count, Value* args) {
	int rand_num = (int)(rand_num_gen(0, 9) + 1);
	char digit = '0' + rand_num;
	return OBJ_VAL(copy_string(&digit, 1));
}

static Value rand_let_native(int arg_count, Value* args) {
	int rand_num = (int)(rand_num_gen(0, 25) + 1);
	char* alph = "abcdefghijklmnopqrstuvwxyz";
	return OBJ_VAL(copy_string(&alph[rand_num], 1));
}

static Value rand_spcc_native(int arg_count, Value* args) {
	int rand_num = (int)(rand_num_gen(0, 9) + 1);
	char* spcc = "!@#$%^&*()";
	return OBJ_VAL(copy_string(&spcc[rand_num], 1));
}

static Value rand_char_native(int arg_count, Value* args) {
	int rand_num = (int)(rand_num_gen(0, 45) + 1);
	char* chars = "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()";
	return OBJ_VAL(copy_string(&chars[rand_num], 1));
}

static Value slice_native(int arg_count, Value* args) {
	int length;
	if ( arg_count == 2 ) {
		length = 1;
	} else if ( arg_count == 3 ) {
		if ( !IS_NUMBER(args[2]) ) {
			runtime_error("The slice function takes the following arguments: str(string), start(int), length(int - optional).");
			return INTERPRET_RUNTIME_ERROR;
		}
		length = AS_NUMBER(args[2]);
	} else {
		runtime_error("The slice function takes the following arguments: str(string), start(int), length(int - optional).");
		return INTERPRET_RUNTIME_ERROR;
	}

	if ( !IS_NUMBER(args[1]) ) {
		runtime_error("The slice function takes the following arguments: str(string), start(int), length(int - optional).");
		return INTERPRET_RUNTIME_ERROR;
	}
	size_t start = AS_NUMBER(args[1]);

	if ( !IS_STRING(args[0]) ) {
		runtime_error("The slice function takes the following arguments: str(string), start(int), length(int - optional).");
		return INTERPRET_RUNTIME_ERROR;
	}
	char* string = AS_CSTRING(args[0]);

	if ( length > UINT64_MAX - 1 ) {
		runtime_error("The slice function cannot create a slice longer than the maximum of uint64 (18446744073709551615).");
	}

	char* string_slice = malloc(sizeof(char) * (length + 1));
	strncpy(string_slice, string + start, length);
	return OBJ_VAL(copy_string(string_slice, length));
}

static Value length_native(int arg_count, Value* args) {
	if ( arg_count != 1 || !IS_STRING(args[0]) ) {
		runtime_error("The length function takes the following argument: str(string).");
		return INTERPRET_RUNTIME_ERROR;
	}

	return NUMBER_VAL(strlen(AS_CSTRING(args[0])));
}

static void reset_stack() {
	vm.stack_top = vm.stack;
	vm.frame_count = 0;
	vm.open_upvalues = NULL;
}

static void runtime_error(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fputs("\n", stderr);

	for ( int i=vm.frame_count - 1; i >= 0; i-- ) {
		CallFrame* frame = &vm.frames[i];
		ObjFunction* function = frame->closure->function;
		size_t instruction = frame->ip - function->chunk.code - 1;
		fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);

		if ( function->name == NULL ) {
			fprintf(stderr, "script\n");
		} else {
			fprintf(stderr, "%s()\n", function->name->chars);
		}
	}

	reset_stack();
}

static void define_native(const char* name, NativeFn function) {
	push(OBJ_VAL(copy_string(name, (int)strlen(name))));
	push(OBJ_VAL(new_native(function)));

	table_set(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);

	pop();
	pop();
}

void init_vm() {
	srand(time(NULL));

	reset_stack();
	vm.objects = NULL;
	vm.bytes_allocated = 0;
	vm.next_gc = 1024 * 1024;

	vm.gray_count = 0;
	vm.gray_capacity = 0;
	vm.gray_stack = NULL;

	init_table(&vm.globals);
	init_table(&vm.strings);

	define_native("clock",      clock_native);
	define_native("lower",      lower_native);
	define_native("upper",      upper_native);
	define_native("rand",       rand_native);
	define_native("rand_int",   rand_int_native);
	define_native("rand_digit", rand_digit_native);
	define_native("rand_let",   rand_let_native);
	define_native("rand_spcc",  rand_spcc_native);
	define_native("rand_char",  rand_char_native);
	define_native("slice",      slice_native);
	define_native("length",     length_native);
}

void push(Value value) {
	*vm.stack_top = value;
	vm.stack_top++;
}

Value pop() {
	vm.stack_top--;
	return *vm.stack_top;
}

static Value peek(int distance) {
	return vm.stack_top[-1 - distance];
}

static bool call(ObjClosure* closure, int arg_count) {
	if ( arg_count != closure->function->arity ) {
		runtime_error("Expected %d arguments but got %d.", closure->function->arity, arg_count);
		return false;
	}

	if ( vm.frame_count == FRAMES_MAX ) {
		runtime_error("Stack overflow.");
		return false;
	}

	CallFrame* frame = &vm.frames[vm.frame_count++];
	frame->closure = closure;
	frame->ip = closure->function->chunk.code;
	frame->slots = vm.stack_top - arg_count - 1;
	return true;
}

static bool call_value(Value callee, int arg_count) {
	if ( IS_OBJ(callee) ) {
		switch ( OBJ_TYPE(callee) ) {
			case OBJ_CLOSURE:
				return call(AS_CLOSURE(callee), arg_count);
			case OBJ_NATIVE: {
				NativeFn native = AS_NATIVE(callee);
				Value result = native(arg_count, vm.stack_top - arg_count);
				vm.stack_top -= arg_count + 1;
				push(result);
				return true;
			}
			default:
				break;
		}
	}

	runtime_error("Can only call functions and classes.");
	return false;
}

static ObjUpvalue* capture_upvalue(Value* local) {
	ObjUpvalue* prev_upvalue = NULL;
	ObjUpvalue* upvalue = vm.open_upvalues;

	while ( upvalue != NULL && upvalue->location > local ) {
		prev_upvalue = upvalue;
		upvalue = upvalue->next;
	}

	if ( upvalue != NULL && upvalue->location == local ) {
		return upvalue;
	}

	ObjUpvalue* created_upvalue = new_upvalue(local);
	created_upvalue->next = upvalue;

	if ( prev_upvalue == NULL ) {
		vm.open_upvalues = created_upvalue;
	} else {
		prev_upvalue->next = created_upvalue;
	}

	return created_upvalue;
}

static void close_upvalues(Value* last) {
	while ( vm.open_upvalues != NULL && vm.open_upvalues->location >= last ) {
		ObjUpvalue* upvalue = vm.open_upvalues;
		upvalue->closed = *upvalue->location;
		upvalue->location = &upvalue->closed;
		vm.open_upvalues = upvalue->next;
	}
}

static bool is_falsey(Value value) {
	return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
	ObjString* b = AS_STRING(peek(0));
	ObjString* a = AS_STRING(peek(1));

	int length = a->length + b->length;
	char* chars = ALLOCATE(char, length + 1);
	memcpy(chars, a->chars, a->length);
	memcpy(chars + a->length, b->chars, b->length);
	chars[length] = '\0';

	ObjString* result = take_string(chars, length);
	pop();
	pop();
	push(OBJ_VAL(result));
}

void free_vm() {
	free_table(&vm.globals);
	free_table(&vm.strings);
	free_objects();
}

static InterpretResult run() {
	CallFrame* frame = &vm.frames[vm.frame_count - 1];

#define READ_BYTE() (*frame->ip++)

#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtime_error("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

#define BITWISE_OP(op) \
	do { \
		if ( IS_BOOL(peek(0)) && IS_BOOL(peek(1)) ) { \
			bool b = AS_BOOL(pop()); \
			bool a = AS_BOOL(pop()); \
			push(BOOL_VAL(a op b)); \
		} else if ( IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)) ) { \
			double b = AS_NUMBER(pop()); \
			double a = AS_NUMBER(pop()); \
			int bi = (int)b; \
			int ai = (int)a; \
			if ( a != ai || b != bi ) { \
				runtime_error("Operands of bitwise operator must be integers not floats."); \
				return INTERPRET_RUNTIME_ERROR; \
			} \
			push(NUMBER_VAL(ai op bi)); \
		} else { \
			runtime_error("Operands of a bitwise operator must be an integer or a boolean."); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
	} while (false)

#define POW_OP() \
	do { \
		if ( !IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)) ) { \
			runtime_error("Operands must be numbers."); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
		double b = AS_NUMBER(pop()); \
		double a = AS_NUMBER(pop()); \
		push(NUMBER_VAL(pow(a, b))); \
	} while (false)

#define MODULO_OP() \
	do { \
		if ( !IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1)) ) { \
			runtime_error("Operands must be numbers."); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
		double b = AS_NUMBER(pop()); \
		double a = AS_NUMBER(pop()); \
		int ai = (int)a; \
		int bi = (int)b; \
		if ( a != ai || b != bi ) { \
			runtime_error("Operands of modulo must be integers."); \
			return INTERPRET_RUNTIME_ERROR; \
		} \
		push(NUMBER_VAL((double)(ai % bi))); \
	} while (false)

	for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
		printf("          ");
		for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
			printf("[ ");
			print_value(*slot);
			printf(" ]");
		}
		printf("\n");
		dissasemble_instruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code));
#endif
		uint8_t instruction;

		switch ( instruction = READ_BYTE() ) {
			case OP_CONSTANT: {
				Value constant = READ_CONSTANT();
				push(constant);
				break;
			}
			case OP_NULL: push(NULL_VAL); break;
			case OP_TRUE: push(BOOL_VAL(true)); break;
			case OP_FALSE: push(BOOL_VAL(false)); break;
			case OP_POP: pop(); break;
			case OP_GET_LOCAL: {
				uint8_t slot = READ_BYTE();
				push(frame->slots[slot]);
				break;
			}
			case OP_GET_GLOBAL: {
				ObjString* name = READ_STRING();
				Value value;

				if ( !table_get(&vm.globals, name, &value) ) {
					runtime_error("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}

				push(value);
				break;
			}
			case OP_DEFINE_GLOBAL: {
				ObjString* name = READ_STRING();
				table_set(&vm.globals, name, peek(0));
				pop();
				break;
			}
			case OP_SET_LOCAL: {
				uint8_t slot = READ_BYTE();
				frame->slots[slot] = peek(0);
				break;
			}
			case OP_SUB_SET_LOCAL: {
				uint8_t slot = READ_BYTE();
				Value initial = frame->slots[slot];
				Value sub = peek(0);

				if ( !IS_NUMBER(initial) || !IS_NUMBER(sub) ) {
					runtime_error("Trying to subtract with '-=' to a variable which is not a number.");
					return INTERPRET_RUNTIME_ERROR;
				}

				frame->slots[slot] = num_to_value(value_to_num(initial) - value_to_num(sub));
				break;
			}
			case OP_ADD_SET_LOCAL: {
				uint8_t slot = READ_BYTE();
				Value initial = frame->slots[slot];
				Value add = peek(0);

				if ( !IS_NUMBER(initial) || !IS_NUMBER(add) ) {
					runtime_error("Trying to add with '+=' to a variable which is not a number.");
					return INTERPRET_RUNTIME_ERROR;
				}

				frame->slots[slot] = num_to_value(value_to_num(initial) + value_to_num(add));
				break;
			}
			case OP_SET_GLOBAL: {
				ObjString* name = READ_STRING();

				if ( table_set(&vm.globals, name, peek(0)) ) {
					table_delete(&vm.globals, name);
					runtime_error("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}

				break;
			}
			case OP_SUB_SET_GLOBAL: {
				ObjString* name = READ_STRING();
				Value initial;
				Value sub = peek(0);

				if ( !table_get(&vm.globals, name, &initial) ) {
					runtime_error("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}

				if ( !IS_NUMBER(initial) || !IS_NUMBER(sub) ) {
					runtime_error("Trying to subtract with '-=' to a variable which is not a number.");
					return INTERPRET_RUNTIME_ERROR;
				}

				Value new_val = num_to_value(value_to_num(initial) - value_to_num(sub));

				if ( table_set(&vm.globals, name, new_val) ) {
					table_delete(&vm.globals, name);
					runtime_error("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}

				break;
			}
			case OP_ADD_SET_GLOBAL: {
				ObjString* name = READ_STRING();
				Value initial;
				Value add = peek(0);

				if ( !table_get(&vm.globals, name, &initial) ) {
					runtime_error("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}

				Value new_val = num_to_value(value_to_num(initial) + value_to_num(add));

				if ( !IS_NUMBER(initial) || !IS_NUMBER(add) ) {
					runtime_error("Trying to add with '+=' to a variable which is not a number.");
					return INTERPRET_RUNTIME_ERROR;
				}

				if ( table_set(&vm.globals, name, new_val) ) {
					table_delete(&vm.globals, name);
					runtime_error("Undefined variable '%s'.", name->chars);
					return INTERPRET_RUNTIME_ERROR;
				}

				break;
			}
			case OP_GET_UPVALUE: {
				uint8_t slot = READ_BYTE();
				push(*frame->closure->upvalues[slot]->location);
				break;
			}
			case OP_SET_UPVALUE: {
				uint8_t slot = READ_BYTE();
				*frame->closure->upvalues[slot]->location = peek(0);
				break;
			}
			case OP_SUB_SET_UPVALUE: {
				uint8_t slot = READ_BYTE();
				Value initial = *frame->closure->upvalues[slot]->location;
				Value sub = peek(0);

				if ( !IS_NUMBER(initial) || !IS_NUMBER(sub) ) {
					runtime_error("Trying to subtract with '-=' to a variable which is not a number.");
					return INTERPRET_RUNTIME_ERROR;
				}

				*frame->closure->upvalues[slot]->location = num_to_value(value_to_num(initial) - value_to_num(sub));
				break;
			}
			case OP_ADD_SET_UPVALUE: {
				uint8_t slot = READ_BYTE();
				Value initial = *frame->closure->upvalues[slot]->location;
				Value add = peek(0);

				if ( !IS_NUMBER(initial) || !IS_NUMBER(add) ) {
					runtime_error("Trying to add with '+=' to a variable which is not a number.");
					return INTERPRET_RUNTIME_ERROR;
				}

				*frame->closure->upvalues[slot]->location = num_to_value(value_to_num(initial) + value_to_num(add));
				break;
			}
			case OP_EQUAL: {
				Value b = pop();
				Value a = pop();
				push(BOOL_VAL(values_equal(a, b)));
				break;
			}
			case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
			case OP_LESSER:   BINARY_OP(BOOL_VAL, <); break;
			case OP_ADD: {
				if ( IS_STRING(peek(0)) && IS_STRING(peek(1)) ) {
					concatenate();
				} else if ( IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)) ) {
					double b = AS_NUMBER(pop());
					double a = AS_NUMBER(pop());
					push(NUMBER_VAL(a + b));
				} else {
					runtime_error("Operands must be two numbers or two strings.");
					return INTERPRET_RUNTIME_ERROR;
				}
				break;
			}
			case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
			case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
			case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
			case OP_MODULO:   MODULO_OP(); break;
			case OP_POW:      POW_OP(); break;
			case OP_ANDB:     BITWISE_OP(&); break;
			case OP_ORB:      BITWISE_OP(|); break;
			case OP_XORB:     BITWISE_OP(^); break;
			case OP_SHIFTR:   BITWISE_OP(>>); break;
			case OP_SHIFTL:   BITWISE_OP(<<); break;
			case OP_NOT:
				push(BOOL_VAL(is_falsey(pop())));
				break;
			case OP_NEGATE: {
				if ( !IS_NUMBER(peek(0)) ) {
					runtime_error("Operand must be a number.");
					return INTERPRET_RUNTIME_ERROR;
				}
				push(NUMBER_VAL(-AS_NUMBER(pop())));
				break;
			}
			case OP_TYPE: {
				print_value(pop());
				printf("\n");
				break;
			}
			case OP_JUMP: {
				uint16_t offset = READ_SHORT();
				frame->ip += offset;
				break;
			}
			case OP_JUMP_IF_FALSE: {
				uint16_t offset = READ_SHORT();
				if ( is_falsey(peek(0)) ) frame->ip += offset;
				break;
			}
			case OP_LOOP: {
				uint16_t offset = READ_SHORT();
				frame->ip -= offset;
				break;
			}
			case OP_CALL: {
				int arg_count = READ_BYTE();
				if ( !call_value(peek(arg_count), arg_count) ) {
					return INTERPRET_RUNTIME_ERROR;
				}
				frame = &vm.frames[vm.frame_count - 1];
				break;
			}
			case OP_CLOSURE: {
				ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
				ObjClosure* closure = new_closure(function);
				push(OBJ_VAL(closure));

				for ( int i=0; i < closure->upvalue_count; i++ ) {
					uint8_t is_local = READ_BYTE();
					uint8_t index = READ_BYTE();

					if ( is_local ) {
						closure->upvalues[i] = capture_upvalue(frame->slots + index);
					} else {
						closure->upvalues[i] = frame->closure->upvalues[index];
					}
				}

				break;
			}
			case OP_CLOSE_UPVALUE:
				close_upvalues(vm.stack_top - 1);
				pop();
				break;
			case OP_RETURN: {
				Value result = pop();
				close_upvalues(frame->slots);
				vm.frame_count--;

				if ( vm.frame_count == 0 ) {
					pop();
					return INTERPRET_OK;
				}

				vm.stack_top = frame->slots;
				push(result);
				frame = &vm.frames[vm.frame_count - 1];
				break;
			}
		}
	}

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
	ObjFunction* function = compile(source);
	if ( function == NULL ) return INTERPRET_COMPILE_ERROR;

	push(OBJ_VAL(function));
	ObjClosure* closure = new_closure(function);
	pop();
	push(OBJ_VAL(closure));
	call(closure, 0);

	return run();
}
