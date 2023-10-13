#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.hpp"

void* andyMalloc(size_t size){
  void* ptr = malloc(size);
  if(ptr == NULL){
    printf("memory allocation failed: %s\n", strerror(errno));
    exit(1);
  }
  return ptr;
}

char* readFileIntoString(const char* filename)
{
  int fd = open(filename, O_RDONLY);
  struct stat buf;
  fstat(fd, &buf);
  off_t size = buf.st_size;

  char* buffer = (char*)andyMalloc(size+1);
  buffer[size] = 0;
  int bytesRead = read(fd, buffer, size);
  if(bytesRead == -1){
    printf("read operation failed: %s\n", strerror(errno));
  }
  if(bytesRead != size){
    printf("reading file \"%s\" mismatched bytes read, expected %d read %d\n", filename, (int)size, bytesRead);
  }
  
  return buffer;
}
