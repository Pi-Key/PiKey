#ifndef FILE_UTILS_H
#define FILE_UTILS_H

struct FileContent {
  char *bytes;
  long size;
};

struct FileContent readFile(const char* path);

#endif
