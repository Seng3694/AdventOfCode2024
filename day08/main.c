#include <aoc/aoc.h>
#include <aoc/filesystem.h>
#include <aoc/mem.h>
#include <stdio.h>

// injected from the Makefile
#ifndef INPUT_HEIGHT
#define INPUT_HEIGHT 1
#endif

#ifndef INPUT_WIDTH
#define INPUT_WIDTH 1
#endif

// with new line for row offsets
#define INPUT_WIDTH_NL (INPUT_WIDTH + 1)

// grep -Po '[a-zA-Z0-9]' day08/input.txt | sort | uniq -c | sort -nk1 | tail -n
// 1 | grep -Po '\d' max amount of antennas of the same type -> 4
#define ANTENNAS_PER_TYPE_MAX 4

#define AOC_T i16
#include <aoc/point.h>
typedef aoc_point2_i16 point;

#define AOC_T point
#define AOC_T_EMPTY ((point){-1, -1})
#define AOC_T_HASH(x) aoc_point2_i16_hash(x)
#define AOC_T_EQUALS(a, b) aoc_point2_i16_equals(a, b)
#define AOC_BASE2_CAPACITY
#include <aoc/set.h>

typedef struct antennas {
  point positions[ANTENNAS_PER_TYPE_MAX];
  u8 count;
} antennas;

#define LETTER_COUNT ('z' - 'a' + 1)
#define DIGIT_COUNT ('9' - '0' + 1)

#define ANTENNA_TYPE_COUNT (LETTER_COUNT + LETTER_COUNT + DIGIT_COUNT)
#define DIGIT_OFFSET 0
#define UPPERCASE_OFFSET DIGIT_COUNT
#define LOWERCASE_OFFSET (DIGIT_COUNT + LETTER_COUNT)

static antennas a[ANTENNA_TYPE_COUNT] = {0};
static aoc_set_point antinodes = {0};

static void parse(char *line, void *ud, const size_t lineNumber) {
  (void)ud;
  for (size_t x = 0; *line; ++x, ++line) {
    const char c = *line;
    if (c >= '0' && c <= '9') {
      antennas *const ant = &a[DIGIT_OFFSET + c - '0'];
      ant->positions[ant->count++] = (point){x, lineNumber};
    } else if (c >= 'A' && c <= 'Z') {
      antennas *const ant = &a[UPPERCASE_OFFSET + c - 'A'];
      ant->positions[ant->count++] = (point){x, lineNumber};
    } else if (c >= 'a' && c <= 'z') {
      antennas *const ant = &a[LOWERCASE_OFFSET + c - 'a'];
      ant->positions[ant->count++] = (point){x, lineNumber};
    }
  }
}

static u32 solve_part1(void) {
  for (size_t i = 0; i < ANTENNA_TYPE_COUNT; ++i) {
    if (a[i].count == 0)
      continue;

    point p;
    for (u8 j = 0; j < a[i].count - 1; ++j) {
      for (u8 k = j + 1; k < a[i].count; ++k) {
        const i16 xdiff = a[i].positions[k].x - a[i].positions[j].x;
        const i16 ydiff = a[i].positions[k].y - a[i].positions[j].y;
        p.x = a[i].positions[j].x - xdiff;
        p.y = a[i].positions[j].y - ydiff;

        if (p.x >= 0 && p.x < INPUT_WIDTH && p.y >= 0 && p.y < INPUT_HEIGHT &&
            !aoc_set_point_contains(&antinodes, p))
          aoc_set_point_insert(&antinodes, p);

        p.x = a[i].positions[k].x + xdiff;
        p.y = a[i].positions[k].y + ydiff;
        if (p.x >= 0 && p.x < INPUT_WIDTH && p.y >= 0 && p.y < INPUT_HEIGHT &&
            !aoc_set_point_contains(&antinodes, p))
          aoc_set_point_insert(&antinodes, p);
      }
    }
  }

  return antinodes.count;
}

static u32 solve_part2(void) {
  for (size_t i = 0; i < ANTENNA_TYPE_COUNT; ++i) {
    if (a[i].count == 0)
      continue;

    for (u8 j = 0; j < a[i].count - 1; ++j) {
      for (u8 k = j + 1; k < a[i].count; ++k) {
        const i16 xdiff = a[i].positions[k].x - a[i].positions[j].x;
        const i16 ydiff = a[i].positions[k].y - a[i].positions[j].y;
        point p = a[i].positions[j];

        while (p.x >= 0 && p.x < INPUT_WIDTH && p.y >= 0 &&
               p.y < INPUT_HEIGHT) {
          if (!aoc_set_point_contains(&antinodes, p))
            aoc_set_point_insert(&antinodes, p);
          p.x -= xdiff;
          p.y -= ydiff;
        }

        p = a[i].positions[k];
        while (p.x >= 0 && p.x < INPUT_WIDTH && p.y >= 0 &&
               p.y < INPUT_HEIGHT) {
          if (!aoc_set_point_contains(&antinodes, p))
            aoc_set_point_insert(&antinodes, p);
          p.x += xdiff;
          p.y += ydiff;
        }
      }
    }
  }

  return antinodes.count;
}

int main(void) {
  aoc_file_read_lines2("day08/input.txt", (aoc_line_num_func)parse, NULL);
  aoc_set_point_create(&antinodes, 1 << 11);

  const u32 part1 = solve_part1();
  aoc_set_point_clear(&antinodes);
  const u32 part2 = solve_part2();

  printf("%u %u\n", part1, part2);

  aoc_set_point_destroy(&antinodes);
}
