#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  for (p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  if (strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), 0, DIRSIZ - strlen(p));
  return buf;
}

void
run_exec(char **cmdargv, int cmdargc, char *matchpath)
{
  char *argv[MAXARG];
  int i;

  for (i = 0; i < cmdargc; i++)
    argv[i] = cmdargv[i];
  argv[i++] = matchpath;
  argv[i] = 0;

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "find: fork failed\n");
    return;
  }
  if (pid == 0) {
    exec(argv[0], argv);
    fprintf(2, "find: exec %s failed\n", argv[0]);
    exit(1);
  } else {
    wait(0);
  }
}

void
find(char *path, char *name, char **cmdargv, int cmdargc)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if ((fd = open(path, 0)) < 0) {
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch (st.type) {
  case T_FILE:
    if (strcmp(fmtname(path), name) == 0) {
      if (cmdargc > 0)
        run_exec(cmdargv, cmdargc, path);
      else
        printf("%s\n", path);
    }
    break;

  case T_DIR:
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      if (stat(buf, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", buf);
        continue;
      }

      if (strcmp(de.name, name) == 0) {
        if (cmdargc > 0) {
          char matchpath[512];
          strcpy(matchpath, buf);
          run_exec(cmdargv, cmdargc, matchpath);
        } else {
          printf("%s\n", buf);
        }
      }

      if (st.type == T_DIR) {
        find(buf, name, cmdargv, cmdargc);
      }
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  char *cmdargv[MAXARG];
  int cmdargc = 0;

  if (argc < 3) {
    fprintf(2, "Usage: find <path> <name> [-exec cmd ...]\n");
    exit(1);
  }

  if (argc > 3) {
    if (strcmp(argv[3], "-exec") != 0) {
      fprintf(2, "Usage: find <path> <name> [-exec cmd ...]\n");
      exit(1);
    }
    for (int i = 4; i < argc; i++) {
      cmdargv[cmdargc++] = argv[i];
    }
  }

  find(argv[1], argv[2], cmdargv, cmdargc);
  exit(0);
}
