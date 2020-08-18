#include "utils.h"

int indent = 0;

void write_sth_to_file(char const *path, char const *content, size_t len) {
  if (len == -1) len = strlen(content);
  int fd;
  xassert((fd = openat(AT_FDCWD, path, O_WRONLY)) >= 0);
  xassert(write(fd, content, len) == len);
  xassert(close(fd) == 0);
}

void deny_to_setgroups() { write_sth_to_file("/proc/self/setgroups", "deny", -1); }

void map_to_root(int id, char const *filename) {
  char buf[0x20] = { 0 };
  int len        = snprintf(buf, 0x20, "0 %d 1", id);
  write_sth_to_file(filename, buf, len);
}