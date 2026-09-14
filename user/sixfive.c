#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
is_separator(char c)
{
  char seps[] = " -\r\t\n./,";
  for (int i = 0; seps[i] != '\0'; i++) {
    if (c == seps[i])
      return 1;
  }
  return 0;
}

void
process(int fd, char *name)
{
  char c;
  int n = 0;
  int have_digits = 0;

  while (read(fd, &c, 1) == 1) {
    if (c >= '0' && c <= '9') {
      n = n * 10 + (c - '0');
      have_digits = 1;
    } else if (is_separator(c)) {
      if (have_digits) {
        if (n % 5 == 0 || n % 6 == 0)
          printf("%d\n", n);
        n = 0;
        have_digits = 0;
      }
    } else {
      // non-digit, non-separator character: reset without printing
      n = 0;
      have_digits = 0;
    }
  }

  // end of file counts as a separator too
  if (have_digits) {
    if (n % 5 == 0 || n % 6 == 0)
      printf("%d\n", n);
  }
}

int
main(int argc, char *argv[])
{
  if (argc == 1) {
    process(0, "stdin");
    exit(0);
  }

  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      fprintf(2, "sixfive: cannot open %s\n", argv[i]);
      continue;
    }
    process(fd, argv[i]);
    close(fd);
  }

  exit(0);
}
