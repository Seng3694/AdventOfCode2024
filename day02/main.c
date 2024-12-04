#include <aoc/aoc.h>
#include <stdio.h>
#include <stdlib.h>

// injected from Makefile
#ifndef INPUT_HEIGHT
#define INPUT_HEIGHT 1
#endif

// awk '{print NF}' day02/input.txt | sort -u | tail -n1
// -> 8
#define MAX_LEVEL_COUNT 8

typedef struct report {
  // grep -Po '\d+' day02/input.txt | sort -un | tail -n1
  // grep -Po '\d+' day02/input.txt | sort -un | head -n1
  //  -> 1 to 99 (can use 8 bit integer. use sign for easier subtractions later)
  i8 levels[MAX_LEVEL_COUNT];
  u8 count;
} report;

static void read_line(char *line, report reports[INPUT_HEIGHT],
                      const size_t lineNumber) {
  AOC_ASSERT(INPUT_HEIGHT > lineNumber);
  u8 count = 0;
  while (*line) {
    reports[lineNumber].levels[count++] = strtoul(line, &line, 10);
    ++line; // skip space or newline
  }
  reports[lineNumber].count = count;
}

static bool is_safe(const report *const r) {
  bool is_increasing = true;
  bool is_decreasing = true;
  for (u8 i = 0; i < r->count - 1; ++i) {
    const i8 signed_diff = r->levels[i] - r->levels[i + 1];
    const u8 abs_diff = abs(signed_diff);
    if (abs_diff < 1 || abs_diff > 3)
      return false;
    is_increasing = is_increasing && signed_diff < 0;
    is_decreasing = is_decreasing && signed_diff > 0;
  }
  return is_increasing || is_decreasing;
}

static bool is_safe2(const report *const r) {
  for (u8 i = 0; i < r->count; ++i) {
    report newReport = {.count = r->count - 1};
    for (u8 j = 0, k = 0; j < r->count; ++j) {
      if (i == j)
        continue;
      newReport.levels[k++] = r->levels[j];
    }
    if (is_safe(&newReport))
      return true;
  }
  return false;
}

static void solve(const report reports[const INPUT_HEIGHT], u32 *const part1,
                  u32 *const part2) {
  u32 p1 = 0, p2 = 0;
  for (u32 i = 0; i < INPUT_HEIGHT; ++i) {
    if (is_safe(&reports[i])) {
      ++p1;
      ++p2;
    } else if (is_safe2(&reports[i])) {
      ++p2;
    }
  }
  *part1 = p1;
  *part2 = p2;
}

int main(void) {
  report reports[INPUT_HEIGHT] = {0};
  aoc_file_read_lines2("day02/input.txt", (aoc_line_num_func)read_line,
                       reports);
  u32 part1, part2;
  solve(reports, &part1, &part2);
  printf("%u %u\n", part1, part2);
}
