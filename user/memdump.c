#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void print_hex64(uint64 val)
{
  char buf[17];
  int i = 16;
  buf[i] = '\0';
  if (val == 0) {
    printf("0\n");
    return;
  }
  while (val > 0) {
    int d = val % 16;
    buf[--i] = (d < 10) ? ('0' + d) : ('a' + d - 10);
    val /= 16;
  }
  printf("%s\n", buf + i);
}
void memdump(char *fmt, char *data, int len)
{
  int off = 0;

  for (int i = 0; fmt[i] != '\0'; i++) {
    char f = fmt[i];
    int need = 0;

    if (f == 'i') need = 4;
    else if (f == 'p') need = 8;
    else if (f == 'h') need = 2;
    else if (f == 'c') need = 1;
    else if (f == 's') need = 8;
    else if (f == 'S') need = 0;  // handled separately, no fixed size

    if (f != 'S' && off + need > len) {
      printf("memdump: not enough data for '%c'\n", f);
      return;
    }

    if (f == 'i') {
      int val;
      memmove(&val, data + off, 4);
      printf("%d\n", val);
      off += 4;
    } else if (f == 'p') {
      uint64 val;
      memmove(&val, data + off, 8);
      print_hex64(val);
      off += 8;
    } else if (f == 'h') {
      unsigned short val;
      memmove(&val, data + off, 2);
      printf("%d\n", val);
      off += 2;
    } else if (f == 'c') {
      printf("%c\n", data[off]);
      off += 1;
    } else if (f == 's') {
      char *strptr;
      memmove(&strptr, data + off, 8);
      printf("%s\n", strptr);
      off += 8;
    } else if (f == 'S') {
      int j = off;
      while (j < len && data[j] != '\0')
        j++;
      write(1, data + off, j - off);
      printf("\n");
      off = len;
    }
  }
}
int
main(int argc, char *argv[])
{
  if (argc == 1) {
    printf("Example 1:\n");
    int a[2] = {61810, 2026};
    memdump("ii", (char *)a, sizeof(a));

    printf("Example 2:\n");
    memdump("S", "a string", sizeof("a string"));

    printf("Example 3:\n");
    char *s = "another";
    memdump("s", (char *)&s, sizeof(s));

    struct sss {
      char *ptr;
      int num1;
      short num2;
      char byte;
      char bytes[8];
    } example;

    example.ptr = "hello";
    example.num1 = 1819438967;
    example.num2 = 100;
    example.byte = 'z';
    strcpy(example.bytes, "xyzzy");

    printf("Example 4:\n");
    memdump("pihcS", (char *)&example, sizeof(example));

    printf("Example 5:\n");
    memdump("sccccc", (char *)&example, sizeof(example));
  } else if (argc == 2) {
    // format in argv[1], up to 512 bytes of data from standard input.
    char data[512];
    int n = 0;
    memset(data, '\0', sizeof(data));
    while (n < sizeof(data)) {
      int nn = read(0, data + n, sizeof(data) - n);
      if (nn <= 0)
        break;
      n += nn;
    }
    memdump(argv[1], data, n);
  } else {
    printf("Usage: memdump [format]\n");
    exit(1);
  }
  exit(0);
}
