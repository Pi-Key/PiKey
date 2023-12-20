#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, object_type) \
	(type*)allocate_object(sizeof(type), object_type)

static Obj* allocate_object(size_t size, ObjType type) {
	Obj* object = (Obj*)reallocate(NULL, 0, size);
	object->type = type;
	object->is_marked = false;

	object->next = vm.objects;
	vm.objects = object;

#ifdef DEBUG_LOG_GC
	printf("%p allocate %zu for %d", (void*)object, size, type);
#endif

	return object;
}

ObjClosure* new_closure(ObjFunction* function) {
	ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalue_count);
	for ( int i=0; i < function->upvalue_count; i++ ) {
		upvalues[i] = NULL;
	}

	ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
	closure->function = function;
	closure->upvalues = upvalues;
	closure->upvalue_count = function->upvalue_count;

	return closure;
}

ObjFunction* new_function() {
	ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
	function->arity = 0;
	function->upvalue_count = 0;
	function->name = NULL;

	init_chunk(&function->chunk);
	return function;
}

ObjNative* new_native(NativeFn function) {
	ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
	native->function = function;
	return native;
}

ObjList* new_list() {
	ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
	list->items = NULL;
	list->count = 0;
	list->capacity = 0;
	return list;
}

void append_to_list(ObjList* list, Value value) {
	if ( list->capacity < list->count + 1 ) {
		int old_capacity = list->capacity;
		list->capacity = GROW_CAPACITY(old_capacity);
		list->items = GROW_ARRAY(Value, list->items, old_capacity, list->capacity);
	}
	
	list->items[list->count] = value;
	list->count++;

	return;
}

void set_in_list(ObjList* list, int index, Value value) {
	if ( index < 0 && abs(index) <= list->count - 1 ) {
		list->items[list->count - 1 - index] = value;
	} else {
		list->items[index] = value;
	}
}

Value value_from_list(ObjList* list, int index) {
	if ( index < 0 && abs(index) <= list->count - 1 ) {
		return list->items[list->count - 1 - index];
	}
	return list->items[index];
}

void delete_from_list(ObjList* list, int index) {
	if ( index < 0 && abs(index) <= list->count - 1 ) {
		index = list->count - 1 - index;
	}

	for ( int i=index; i < list->count - 1; i++ ) {
		list->items[i] = list->items[i+1];
	}

	list->items[list->count - 1] = NULL_VAL;
	list->count --;
}

static ObjString* allocate_string(char* chars, int length, uint32_t hash) {
	ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
	string->length = length;
	string->chars = chars;
	string->hash = hash;

	push(OBJ_VAL(string));
	table_set(&vm.strings, string, NULL_VAL);
	pop();

	return string;
}

static uint32_t hash_string(const char* key, int length) {
	uint32_t hash = 2166136261u;
	for ( int i=0; i < length; i++ ) {
		hash ^= (uint8_t)key[i];
		hash *= 16777619;
	}

	return hash;
}

ObjString* take_string(char* chars, int length) {
	uint32_t hash = hash_string(chars, length);

	ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
	if ( interned != NULL ) {
		FREE_ARRAY(char, chars, length + 1);
		return interned;
	}

	return allocate_string(chars, length, hash);
}

ObjString* copy_string(const char* chars, int length) {
	uint32_t hash = hash_string(chars, length);

	ObjString* interned = table_find_string(&vm.strings, chars, length, hash);
	if ( interned != NULL ) return interned;

	char* heap_chars = ALLOCATE(char, length + 1);
	memcpy(heap_chars, chars, length);
	heap_chars[length] = '\0';
	return allocate_string(heap_chars, length, hash);
}

ObjUpvalue* new_upvalue(Value* slot) {
	ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
	upvalue->closed = NULL_VAL;
	upvalue->location = slot;
	upvalue->next = NULL;
	return upvalue;
}

static void print_function(ObjFunction* function) {
	if ( function->name == NULL ) {
		printf("<script>");
		return;
	}
	printf("<fn %s>", function->name->chars);
}

static void print_list(ObjList* list) {
	printf("[%d", (int)AS_NUMBER(list->items[0]));
	for ( int i=1; i < list->count; i++) {
		printf(", ");
		printf("%d", (int)AS_NUMBER(list->items[i]));
	}
	printf("]");
}

void print_object(Value value) {
	switch ( OBJ_TYPE(value) ) {
		case OBJ_CLOSURE:
			print_function(AS_CLOSURE(value)->function);
			break;
		case OBJ_FUNCTION:
			print_function(AS_FUNCTION(value));
			break;
		case OBJ_NATIVE:
			printf("<native fn>");
			break;
		case OBJ_STRING:
			printf("%s", AS_CSTRING(value));
			break;
		case OBJ_UPVALUE:
			printf("upvalue");
			break;
		case OBJ_LIST:
			print_list(AS_LIST(value));
			break;
	}
}
