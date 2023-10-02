#include "./helpers/pk_errors.h"
#include "./helpers/pk_help.h"
#include "./parser/parser.h"

int main(int argc, char* argv[]) {
  if (argc < 2 ) {
    pk_error("You must supply at least one argument");
    pk_help();

    return 22;
  } else if (argc == 2) {
    file_parser(argv[1]);
  } else {
    pk_error("Not implemented yet.");
    return 1;
  }

  return 0;
}

