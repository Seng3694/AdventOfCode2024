#include <aoc/aoc.h>
#include <stdlib.h>
#include <stdio.h>

#define LOCATION_COUNT 1000

typedef struct context {
  i32 left[LOCATION_COUNT];
  i32 right[LOCATION_COUNT];
} context;

static inline void skip_spaces(char *line, char **output) {
  while (*line == ' ')
    ++line;
  *output = line;
}

static void read_line(char *line, context *ctx, const size_t lineNumber) {
  AOC_ASSERT(LOCATION_COUNT > lineNumber);
  ctx->left[lineNumber] = strtoul(line, &line, 10);
  skip_spaces(line, &line);
  ctx->right[lineNumber] = strtoul(line, NULL, 10);
}

static inline i32 compare(const i32 *const left, const i32 *const right) {
  return *left - *right;
}

static u64 solve_part1(const context *const ctx) {
  u64 solution = 0;
  for (u32 i = 0; i < LOCATION_COUNT; ++i)
    solution += abs(ctx->left[i] - ctx->right[i]);
  return solution;
}

static u64 solve_part2(const context *const ctx) {
  // assume sorted input in descending order
  u64 score = 0;
  const i32 *left = ctx->left;
  const i32 *right = ctx->right;

  u32 l = 0, r = 0;
  u32 count = 0;
  while (r < LOCATION_COUNT && l < LOCATION_COUNT) {
    if (left[l] == right[r]) {
      i32 v = left[l];
      count = 0;
      // count right side
      while (r < LOCATION_COUNT && v == right[r]) {
        ++r;
        ++count;
      }
      // multiply with count left side
      while (l < LOCATION_COUNT && left[l] == v) {
        score += count * v;
        ++l;
      }
    }

    // synchronize
    while (r < LOCATION_COUNT && l < LOCATION_COUNT && left[l] != right[r]) {
      while (r < LOCATION_COUNT && left[l] > right[r])
        ++r;
      while (r < LOCATION_COUNT && l < LOCATION_COUNT && right[r] > left[l])
        ++l;
    }
  }
  return score;
}

int main(void) {
  context ctx = {0};
  aoc_file_read_lines2("day01/input.txt", (aoc_line_num_func)read_line, &ctx);
  qsort(ctx.left, LOCATION_COUNT, sizeof(u32), (__compar_fn_t)compare);
  qsort(ctx.right, LOCATION_COUNT, sizeof(u32), (__compar_fn_t)compare);

  const u64 part1 = solve_part1(&ctx);
  const u64 part2 = solve_part2(&ctx);

  printf("%lu %lu\n", part1, part2);
}
