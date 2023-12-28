#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

#include "hid.h"

#define FLAG_VALUE 123

void core1_entry() {
	multicore_fifo_push_blocking(FLAG_VALUE);
	uint32_t g = multicore_fifo_pop_blocking();
 
	if (g != FLAG_VALUE) {
		printf("An error occured on core 1.");
		exit(1);
	}

	start_hid();
}

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
	stdio_init_all();
	if ( cyw43_arch_init() ) { // TODO: Detect if has wifi support or not and remove imports / running wifi commands (will only be fixed after functional pico w)
		printf("An error occured while trying to initialize wifi");
		exit(1);
	}

	multicore_launch_core1(core1_entry);
	uint32_t g = multicore_fifo_pop_blocking();

	if ( g != FLAG_VALUE ) {
		printf("An error occured on core 0.");
		exit(1);
	} else {
		multicore_fifo_push_blocking(FLAG_VALUE);
	}

	sleep_ms(1000);
	send_word("TESTING");

	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
	init_vm();

	// if ( argc == 2 ) {
	// 	run_file(argv[1]);
	// } else {
	// 	fprintf(stderr, "Usage: pikey [path]\n");
	// 	exit(64);
	// }

	free_vm();
	return 0;
}
