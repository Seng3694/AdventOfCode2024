#include <aoc/aoc.h>
#include <aoc/filesystem.h>
#include <aoc/mem.h>
#include <stdio.h>
#include <memory.h>

// both injected from the Makefile
#ifndef INPUT_HEIGHT
#define INPUT_HEIGHT 1
#endif

#ifndef INPUT_WIDTH
#define INPUT_WIDTH 1
#endif

// with new line for row offsets
#define INPUT_WIDTH_NL (INPUT_WIDTH + 1)

typedef enum direction {
  DIRECTION_UP,
  DIRECTION_UP_RIGHT,
  DIRECTION_RIGHT,
  DIRECTION_DOWN_RIGHT,
  DIRECTION_DOWN,
  DIRECTION_DOWN_LEFT,
  DIRECTION_LEFT,
  DIRECTION_UP_LEFT,
  DIRECTION_COUNT,
} direction;

static const i32 direction_offsets[DIRECTION_COUNT] = {
    [DIRECTION_UP] = -INPUT_WIDTH_NL,
    [DIRECTION_UP_RIGHT] = -INPUT_WIDTH_NL + 1,
    [DIRECTION_RIGHT] = 1,
    [DIRECTION_DOWN_RIGHT] = INPUT_WIDTH_NL + 1,
    [DIRECTION_DOWN] = INPUT_WIDTH_NL,
    [DIRECTION_DOWN_LEFT] = INPUT_WIDTH_NL - 1,
    [DIRECTION_LEFT] = -1,
    [DIRECTION_UP_LEFT] = -INPUT_WIDTH_NL - 1,
};

typedef bool (*bounds_check)(const i32 x, const i32 y, const u32 len);

static inline bool bounds_check_empty(const i32 x, const i32 y, const u32 len) {
  (void)x;
  (void)y;
  (void)len;
  return true;
}

static inline bool bounds_check_up(const i32 x, const i32 y, const u32 len) {
  (void)x;
  return y - (i32)len >= 0;
}

static inline bool bounds_check_right(const i32 x, const i32 y, const u32 len) {
  (void)y;
  return x + len < INPUT_WIDTH;
}

static inline bool bounds_check_down(const i32 x, const i32 y, const u32 len) {
  (void)x;
  return y + len < INPUT_HEIGHT;
}

static inline bool bounds_check_left(const i32 x, const i32 y, const u32 len) {
  (void)y;
  return x - (i32)len >= 0;
}

static const bounds_check direction_bounds_checks[DIRECTION_COUNT][2] = {
    [DIRECTION_UP] = {bounds_check_empty, bounds_check_up},
    [DIRECTION_UP_RIGHT] = {bounds_check_right, bounds_check_up},
    [DIRECTION_RIGHT] = {bounds_check_right, bounds_check_empty},
    [DIRECTION_DOWN_RIGHT] = {bounds_check_right, bounds_check_down},
    [DIRECTION_DOWN] = {bounds_check_empty, bounds_check_down},
    [DIRECTION_DOWN_LEFT] = {bounds_check_left, bounds_check_down},
    [DIRECTION_LEFT] = {bounds_check_left, bounds_check_empty},
    [DIRECTION_UP_LEFT] = {bounds_check_left, bounds_check_up},
};

static u8 check_part1(const char *const input, const i32 x, const i32 y) {
  u8 count = 0;
  AOC_ASSERT(input[y * INPUT_WIDTH_NL + x] == 'X');
  for (direction d = 0; d < DIRECTION_COUNT; ++d) {
    if (!direction_bounds_checks[d][0](x, y, 3) ||
        !direction_bounds_checks[d][1](x, y, 3))
      continue;

    for (i32 i = 1, j = y * INPUT_WIDTH_NL + x; i < 4; ++i) {
      j += direction_offsets[d];
      if (input[j] != "XMAS"[i])
        goto next;
    }

    ++count;
  next:;
  }

  return count;
}

static u8 check_part2(const char *const input, const i32 x, const i32 y) {
  if (!bounds_check_up(x, y, 1) || !bounds_check_right(x, y, 1) ||
      !bounds_check_down(x, y, 1) || !bounds_check_left(x, y, 1))
    return false;

  AOC_ASSERT(input[y * INPUT_WIDTH_NL + x] == 'A');
  const char tl = input[(y - 1) * INPUT_WIDTH_NL + (x - 1)];
  const char tr = input[(y - 1) * INPUT_WIDTH_NL + (x + 1)];
  const char br = input[(y + 1) * INPUT_WIDTH_NL + (x + 1)];
  const char bl = input[(y + 1) * INPUT_WIDTH_NL + (x - 1)];
  // check if the two diagonals have MS or SM
#define IS_MS(a, b) ((((a) ^ (b)) ^ ('M' ^ 'S')) == 0)
  return IS_MS(tl, br) && IS_MS(bl, tr);
#undef IS_MS
}

static u32 solve(const char *const input,
                 u8 (*check)(const char *const, const i32, const i32),
                 const char start) {
  u32 solution = 0;
  for (i32 y = 0; y < INPUT_HEIGHT; ++y) {
    const u32 yOffset = y * INPUT_WIDTH_NL;
    for (i32 x = 0; x < INPUT_WIDTH; ++x) {
      if (input[yOffset + x] == start) {
        solution += check(input, x, y);
      }
    }
  }
  return solution;
}

int main(void) {
  char *input = aoc_file_read_all2("day04/input.txt");
  const u32 part1 = solve(input, check_part1, 'X');
  const u32 part2 = solve(input, check_part2, 'A');
  aoc_free(input);
  printf("%u %u\n", part1, part2);
}
