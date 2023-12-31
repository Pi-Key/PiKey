#ifndef pikey_object_h
#define pikey_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_CLOSURE(value)  is_obj_type(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_NATIVE(value)   is_obj_type(value, OBJ_NATIVE)
#define IS_STRING(value)   is_obj_type(value, OBJ_STRING)
#define IS_LIST(value)     is_obj_type(value, OBJ_LIST)

#define AS_CLOSURE(value)  ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)   (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)   ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)  (((ObjString*)AS_OBJ(value))->chars)
#define AS_LIST(value)     ((ObjList*)AS_OBJ(value))

typedef enum {
	OBJ_CLOSURE,
	OBJ_FUNCTION,
	OBJ_NATIVE,
	OBJ_STRING,
	OBJ_UPVALUE,
	OBJ_LIST
} ObjType;

struct Obj {
	ObjType type;
	bool is_marked;
	struct Obj* next;
};

typedef struct {
	Obj obj;
	int arity;
	int upvalue_count;
	Chunk chunk;
	ObjString* name;
} ObjFunction;

typedef Value (*NativeFn)(int arg_count, Value* args);

typedef struct {
	Obj obj;
	NativeFn function;
} ObjNative;

struct ObjString {
	Obj obj;
	int length;
	char* chars;
	uint32_t hash;
};

typedef struct {
	Obj obj;
	int count;
	int capacity;
	Value* items;
} ObjList;

typedef struct ObjUpvalue {
	Obj obj;
	Value* location;
	Value closed;
	struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct {
	Obj obj;
	ObjFunction* function;
	ObjUpvalue** upvalues;
	int upvalue_count;
} ObjClosure;

ObjClosure* new_closure(ObjFunction* function);

ObjFunction* new_function();

ObjNative* new_native(NativeFn);

ObjList* new_list();
void append_to_list(ObjList* list, Value value);
void set_in_list(ObjList* list, int index, Value value);
Value value_from_list(ObjList* list, int index);
void delete_from_list(ObjList* list, int index);

void set_in_string(ObjString* string, int index, char character);
ObjString* take_string(char* chars, int length);
ObjString* copy_string(const char* chars, int length);

ObjUpvalue* new_upvalue(Value* slot);

void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type) {
	return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif // !pikey_object_h
