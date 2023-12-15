#ifndef pikey_compiler_h
#define pikey_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction* compile(const char* source);

void mark_compiler_roots();

#endif // !pikey_compiler_h
