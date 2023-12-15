#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

static char* read_file(const char* path) {
	FILE* file = fopen(path, "rb");

	if ( file == NULL ) {
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	size_t file_size = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(file_size + 1);
	size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
	if ( buffer == NULL ) {
		fprintf(stderr, "Could not read file \"%s\".\n", path);
		exit(74);
	}

	buffer[bytes_read] = '\0';

	fclose(file);
	return buffer;
}

static void run_file(const char* path) {
	char* source = read_file(path);
	InterpretResult result = interpret(source);
	free(source);

	switch ( result ) {
		case INTERPRET_COMPILE_ERROR: exit(65);
		case INTERPRET_RUNTIME_ERROR: exit(70);
		default:;
	}
}

int main(int argc, char* argv[]) {
	init_vm();

	if ( argc == 2 ) {
		run_file(argv[1]);
	} else {
		fprintf(stderr, "Usage: pikey [path]\n");
		exit(64);
	}

	free_vm();
	return 0;
}
