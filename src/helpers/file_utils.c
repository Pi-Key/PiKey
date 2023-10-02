#include "file_utils.h"
#include "pk_errors.h"
#include <stdio.h>
#include <stdlib.h>

struct FileContent readFile(const char* file_path) {
  struct FileContent content = { NULL, 0 };

  FILE* fptr = fopen(file_path, "rb");
  if ( fptr == NULL ) {
    pk_error("File could not be openend!");
    return content;
  }

  fseek(fptr, 0, SEEK_END);
  content.size = ftell(fptr);
  fseek(fptr, 0, SEEK_SET);

  content.bytes = (char *)malloc(content.size);
  if ( content.bytes == NULL ) {
    pk_error("Error when allocating memory for reading file!");
    fclose(fptr);
    return content;
  }

  fread(content.bytes, 1, content.size, fptr);
  fclose(fptr);

  return content;
}
