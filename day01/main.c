#include <aoc/aoc.h>
#include <stdlib.h>
#include <stdio.h>

// injected from Makefile
#ifndef INPUT_HEIGHT
#define INPUT_HEIGHT 1
#endif

typedef struct context {
  i32 left[INPUT_HEIGHT];
  i32 right[INPUT_HEIGHT];
} context;

static inline void skip_spaces(char *line, char **output) {
  while (*line == ' ')
    ++line;
  *output = line;
}

static void read_line(char *line, context *ctx, const size_t lineNumber) {
  AOC_ASSERT(INPUT_HEIGHT > lineNumber);
  ctx->left[lineNumber] = strtoul(line, &line, 10);
  skip_spaces(line, &line);
  ctx->right[lineNumber] = strtoul(line, NULL, 10);
}

static inline i32 compare(const i32 *const left, const i32 *const right) {
  return *left - *right;
}

static u64 solve_part1(const context *const ctx) {
  u64 solution = 0;
  for (u32 i = 0; i < INPUT_HEIGHT; ++i)
    solution += abs(ctx->left[i] - ctx->right[i]);
  return solution;
}

static u64 solve_part2(const context *const ctx) {
  // assume sorted input
  u64 score = 0;
  const i32 *left = ctx->left;
  const i32 *right = ctx->right;

  u32 l = 0, r = 0;
  u32 count = 0;
  while (r < INPUT_HEIGHT && l < INPUT_HEIGHT) {
    if (left[l] == right[r]) {
      i32 v = left[l];
      count = 0;
      // count right side
      while (r < INPUT_HEIGHT && v == right[r]) {
        ++r;
        ++count;
      }
      // multiply with count left side
      while (l < INPUT_HEIGHT && left[l] == v) {
        score += count * v;
        ++l;
      }
    }

    // synchronize
    while (r < INPUT_HEIGHT && l < INPUT_HEIGHT && left[l] != right[r]) {
      while (r < INPUT_HEIGHT && left[l] > right[r])
        ++r;
      while (r < INPUT_HEIGHT && l < INPUT_HEIGHT && right[r] > left[l])
        ++l;
    }
  }
  return score;
}

int main(void) {
  context ctx = {0};
  aoc_file_read_lines2("day01/input.txt", (aoc_line_num_func)read_line, &ctx);
  qsort(ctx.left, INPUT_HEIGHT, sizeof(u32), (__compar_fn_t)compare);
  qsort(ctx.right, INPUT_HEIGHT, sizeof(u32), (__compar_fn_t)compare);
  printf("%lu %lu\n", solve_part1(&ctx), solve_part2(&ctx));
}
