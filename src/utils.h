#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define xassert(action)                                                                                                                              \
  do {                                                                                                                                               \
    if (action) break;                                                                                                                               \
    perror("\033[31m" #action "\033[1;5m");                                                                                                          \
    fprintf(stderr, "\033[0m");                                                                                                                      \
    exit(EXIT_FAILURE);                                                                                                                              \
  } while (0)

#define uassert(action, message)                                                                                                                     \
  do {                                                                                                                                               \
    if (action) break;                                                                                                                               \
    fprintf(stderr, "%*s\033[31m" #action "\033[1;5m: " message "\033[0m\n", (indent++) * 4, "");                                                    \
    exit(EXIT_FAILURE);                                                                                                                              \
  } while (0)

extern int indent;

#define DEBUG_ASSERT(x)                                                                                                                              \
  do {                                                                                                                                               \
    printf("%*s\033[1m[+]\033[0m\033[37m %s\n", (indent++) * 4, "", #x);                                                                             \
    xassert(x);                                                                                                                                      \
    indent--;                                                                                                                                        \
  } while (0)

#define DEBUG_EXEC(x)                                                                                                                                \
  do {                                                                                                                                               \
    printf("%*s\033[1m[+]\033[0m\033[90m %s\033[0m\n", (indent++) * 4, "", #x);                                                                      \
    x;                                                                                                                                               \
    indent--;                                                                                                                                        \
  } while (0)

#define DEBUG_BLOCK(text, ...)                                                                                                                       \
  do {                                                                                                                                               \
    printf("%*s\033[1m[+]\033[32m %s\033[0m\n", (indent++) * 4, "", text);                                                                           \
    __VA_ARGS__;                                                                                                                                     \
    indent--;                                                                                                                                        \
  } while (0)

#define DEBUG_PRINTF(format, ...) printf("%*s\033[1m[-]\033[0m\033[34m " format "\033[0m", indent * 4, "", ##__VA_ARGS__)

void write_sth_to_file(char const *path, char const *content, size_t len);
void deny_to_setgroups();
void map_to_root(int id, char const *filename);