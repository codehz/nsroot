#include "utils.h"

typedef struct strlist {
  struct strlist *next;
  char data[0];
} strlist;

void printLogo();
void printUsage(char const *);
int mkpath(char *dir, mode_t mode);

pid_t pid = 0;
void sig_handler(int signo) { return; }

static const char *optString = "xXpfr:t:b:c:w:a:ph?";

int main(int argc, char **argv) {
  printLogo();
  int ceuid      = geteuid();
  int cegid      = getegid();
  char *root     = NULL;
  char *work     = NULL;
  char *appname  = NULL;
  int mount_proc = 0;
  int do_fork    = 0;
  int opt;
  char const *progname = argv[0];
  DEBUG_BLOCK("setup environment", while ((opt = getopt(argc, argv, optString)) != -1) {
    switch (opt) {
    case 'x':
      DEBUG_BLOCK("setup user & mount namespace", {
        DEBUG_ASSERT(unshare(CLONE_NEWUSER | CLONE_NEWNS) == 0);
        DEBUG_EXEC(deny_to_setgroups());
        DEBUG_EXEC(map_to_root(ceuid, "/proc/self/uid_map"));
        DEBUG_EXEC(map_to_root(cegid, "/proc/self/gid_map"));
      });
      break;
    case 'X': DEBUG_BLOCK("setup mount namespace", { DEBUG_ASSERT(unshare(CLONE_NEWNS) == 0); }); break;
    case 'r':
      DEBUG_PRINTF("newroot set: %s\n", optarg);
      root = optarg;
      break;
    case 'w':
      DEBUG_PRINTF("workdir set: %s\n", optarg);
      work = optarg;
      break;
    case 'p':
      DEBUG_PRINTF("mount proc set\n");
      mount_proc = 1;
      break;
    case 'f':
      DEBUG_PRINTF("fork set\n");
      do_fork = 1;
      break;
    case 'a':
      DEBUG_PRINTF("appname set: %s\n", optarg);
      appname = strdup(optarg);
      break;
    case 'b':
      DEBUG_BLOCK("mount bind", {
        char from[PATH_MAX]       = { 0 };
        char to[PATH_MAX]         = { 0 };
        char redirected[PATH_MAX] = { 0 };
        if (sscanf(optarg, "%[^:]:%[^:]", from, to) != 2) {
          strcpy(from, optarg);
          strcpy(to, optarg);
        }
        uassert(strlen(from) && strlen(to), "bind target and source need to be non-empty strings.");
        uassert(to[0] == '/', "bind target need to be an absolute path.");
        if (root && strlen(root)) {
          strcat(redirected, root);
          strcat(redirected, root[strlen(root) - 1] == '/' ? to + 1 : to);
        } else {
          strcpy(redirected, to);
        }
        DEBUG_PRINTF("bind %s -> %s\n", from, redirected);
        xassert(mount(from, redirected, "tmpfs", MS_BIND | MS_REC, NULL) == 0);
      });
      break;
    case 't':
      DEBUG_BLOCK("mount tmpfs", {
        uassert(strlen(optarg), "tmpdir need to be a non-empty string.");
        uassert(optarg[0] == '/', "tmpdir need to be an absolute path.");
        char path[PATH_MAX]       = { 0 };
        char redirected[PATH_MAX] = { 0 };
        char options[64]          = { 0 };
        if (sscanf(optarg, "%[^:]:%s", path, options) != 2) {
          strcpy(path, optarg);
          options[0] = 0;
        }
        if (root && strlen(root)) {
          strcat(redirected, root);
          strcat(redirected, root[strlen(root) - 1] == '/' ? path + 1 : path);
        } else {
          strcpy(redirected, path);
        }
        DEBUG_PRINTF("mktmpfs %s %s\n", redirected, options);
        xassert(mount("tmpfs", redirected, "tmpfs", 0, options) == 0);
      });
      break;
    case 'c':
      DEBUG_BLOCK("create directory structures", {
        uassert(strlen(optarg), "directory name need to be a non-empty string.");
        uassert(optarg[0] == '/', "directory name need to be an absolute path.");
        char redirected[PATH_MAX] = { 0 };
        if (root && strlen(root)) {
          strcat(redirected, root);
          strcat(redirected, root[strlen(root) - 1] == '/' ? optarg + 1 : optarg);
        } else {
          strcpy(redirected, optarg);
        }
        DEBUG_PRINTF("mkdir %s\n", redirected);
        xassert(mkpath(redirected, 0755) == 0);
      });
      break;
    case 'h':
    case '?':
    default:
      printUsage(progname);
      exit(EXIT_FAILURE);
      break;
    }
  });
  if (optind >= argc) {
    fprintf(stderr, "Expected argument after options\n");
    exit(EXIT_FAILURE);
  }
  DEBUG_BLOCK("addition steps", {
    if (root) {
      DEBUG_BLOCK("chroot", {
        DEBUG_PRINTF("chroot %s\n", root);
        xassert(chroot(root) == 0);
      });
    }
    if (work) {
      DEBUG_BLOCK("chdir", {
        DEBUG_PRINTF("chdir %s\n", work);
        xassert(chdir(work) == 0);
      });
    }
    if (do_fork) {
      if (mount_proc) {
        DEBUG_BLOCK("setup pid namespace", { DEBUG_ASSERT(unshare(CLONE_NEWPID) == 0); });
      }
      DEBUG_BLOCK("fork", {
        xassert((pid = fork()) >= 0);
        if (pid == 0) {
          if (mount_proc) {
            DEBUG_BLOCK("mount proc", { xassert(mount("none", "/proc", "proc", 0, NULL) == 0); });
          }
          goto exec;
        } else {
          for (int i = 1; i < SIGRTMAX; i++) signal(i, sig_handler);
          waitpid(pid, NULL, 0);
          DEBUG_PRINTF("process %d exited.\n", pid);
        }
      });
    } else
      goto exec;
    return 0;
  exec:
    if (appname) {
      int idx = optind + 1;
      while (argv[idx]) idx++;
      char *cpargv[idx - optind + 1];
      cpargv[0] = appname;
      for (int i = 1; i < idx - optind; i++) cpargv[i] = argv[optind + i];
      cpargv[idx - optind] = 0;
      DEBUG_BLOCK("execvp", { xassert(execvp(argv[optind], cpargv) == 0); });
    } else
      DEBUG_BLOCK("execvp", { xassert(execvp(argv[optind], &argv[optind]) == 0); });
  });
}

void printUsage(char const *name) {
  printf("%s [-x] [-X] [-p] [-f] [-r root] [-b dir[:target]] [-t target[:options]] [-c target] [-w workdir] /path/to/program [...args]\n", name);
  puts("usage:");
  puts("-x\n\tSetup user & mount namespace.");
  puts("-X\n\tSetup mount namespace.");
  puts("-p\n\tMount proc filesystem.");
  puts("-f\n\tFork before exec.");
  puts("-r path\n\tUse *path* as the new guest root file-system, default is `/`.");
  puts("-b path\n\tMake the content of *path* accessible in the guest rootfs.");
  puts("-t path\n\tCreate tmpfs on *path* in guest rootfs.");
  puts("-c path\n\tCreate folder structure in guest rootfs.");
  puts("-w path\n\tSet the initial working directory to *path*.");
  puts("-a appname\n\tSet the initial process name.");
}

int mkpath(char *dir, mode_t mode) {
  struct stat sb;
  if (!dir) {
    errno = EINVAL;
    return 1;
  }
  if (!stat(dir, &sb)) return 0;
  if (mkpath(dirname(strdupa(dir)), mode)) return 1;
  return mkdir(dir, mode);
}