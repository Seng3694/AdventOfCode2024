#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct parser {
  char *current;
  char *start;
  size_t len;
} parser;

static bool match(parser *const p, const char expected) {
  if (*p->current == '\0')
    return false;
  if (*p->current != expected)
    return false;
  ++p->current;
  return true;
}

static bool match_string(parser *const p, const int start, const int length,
                         const char *rest) {
  if ((p->current - p->start) + start + length >= p->len)
    return false;

  if (memcmp(p->current + start, rest, length) != 0)
    return false;

  p->current += (start + length);
  return true;
}

static void parse(char *const input, const size_t len, u64 *const part1,
                  u64 *const part2) {
  parser p = {.start = input, .current = input, .len = len};
  u64 p1 = 0;
  u64 p2 = 0;
  bool do_mul = true;

  while (*p.current) {
    switch (*p.current) {
    case 'd':
      if (match_string(&p, 1, 6, "on't()")) {
        do_mul = false;
      } else if (match_string(&p, 1, 3, "o()")) {
        do_mul = true;
      } else {
        ++p.current;
      }
      break;
    case 'm':
      if (match_string(&p, 1, 3, "ul(")) {
        if (!isdigit(*p.current))
          break;

        const u32 left = strtoul(p.current, &p.current, 10);

        if (!match(&p, ','))
          break;

        if (!isdigit(*p.current))
          break;

        const u32 right = strtoul(p.current, &p.current, 10);

        if (!match(&p, ')'))
          break;

        const u32 result = left * right;
        p1 += result;
        p2 += (result * (u32)do_mul);
      } else {
        ++p.current;
      }
      break;
    default:
      ++p.current;
      break;
    }
  }

  *part1 = p1;
  *part2 = p2;
}

int main(void) {
  char *input = NULL;
  size_t len = 0;
  aoc_file_read_all1("day03/input.txt", &input, &len);

  u64 part1, part2;
  parse(input, len, &part1, &part2);

  printf("%lu %lu\n", part1, part2);

  aoc_free(input);
}
