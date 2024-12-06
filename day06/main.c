#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <aoc/filesystem.h>
#include <stdio.h>

// both injected from the Makefile
#ifndef INPUT_HEIGHT
#define INPUT_HEIGHT 1
#endif

#ifndef INPUT_WIDTH
#define INPUT_WIDTH 1
#endif

// with new line for row offsets
#define INPUT_WIDTH_NL (INPUT_WIDTH + 1)

#define AOC_T u8
#include <aoc/point.h>
typedef aoc_point2_u8 point;

#define AOC_T point
#define AOC_T_EMPTY ((point){0xff, 0xff})
#define AOC_T_HASH(x) aoc_point2_u8_hash(x)
#define AOC_T_EQUALS(a, b) aoc_point2_u8_equals(a, b)
#define AOC_BASE2_CAPACITY
#include <aoc/set.h>

typedef enum direction {
  DIRECTION_UP,
  DIRECTION_RIGHT,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
} direction;

typedef struct guard_data {
  point pos;
  direction dir : 4;
} guard_data;

static inline u32 guard_data_hash(const guard_data *const d) {
  const u32 hash = aoc_point2_u8_hash(&d->pos);
  return ((hash << 5) + hash) + d->dir;
}

static inline bool guard_data_equals(const guard_data *const left,
                                     const guard_data *const right) {
  return left->pos.x == right->pos.x && left->pos.y == right->pos.y &&
         left->dir == right->dir;
}

#define AOC_T guard_data
#define AOC_T_EMPTY ((guard_data){.pos = {0xff, 0xff}, .dir = DIRECTION_UP})
#define AOC_T_HASH(x) guard_data_hash(x)
#define AOC_T_EQUALS(a, b) guard_data_equals(a, b)
#define AOC_BASE2_CAPACITY
#include <aoc/set.h>

static const point move_offsets[4] = {
    [DIRECTION_UP] = {0, -1},
    [DIRECTION_RIGHT] = {1, 0},
    [DIRECTION_DOWN] = {0, 1},
    [DIRECTION_LEFT] = {-1, 0},
};

static point get_start_point(const char *const input) {
  for (u8 y = 0; y < INPUT_HEIGHT; ++y) {
    for (u8 x = 0; x < INPUT_WIDTH; ++x) {
      if (input[y * INPUT_WIDTH_NL + x] == '^')
        return (point){x, y};
    }
  }
  return (point){0};
}

#define BOUNDS_CHECK(p)                                                        \
  ((p).x >= 0 && (p).x < INPUT_WIDTH && (p).y >= 0 && (p).y < INPUT_HEIGHT)

static inline void move(const char *const input, guard_data *const gd) {
  point new_pos = {
      gd->pos.x + move_offsets[gd->dir].x,
      gd->pos.y + move_offsets[gd->dir].y,
  };

  size_t i = new_pos.y * INPUT_WIDTH_NL + new_pos.x;

  while (BOUNDS_CHECK(new_pos) && input[i] == '#') {
    gd->dir = (gd->dir + 1) & 3;
    new_pos.x = gd->pos.x + move_offsets[gd->dir].x;
    new_pos.y = gd->pos.y + move_offsets[gd->dir].y;
    i = new_pos.y * INPUT_WIDTH_NL + new_pos.x;
  }

  gd->pos = new_pos;
}

static bool check_loop(const char *const input, const point start,
                       const direction start_dir,
                       aoc_set_guard_data *const loop_set) {
  guard_data gd = {start, start_dir};
  aoc_set_guard_data_clear(loop_set);

  while (BOUNDS_CHECK(gd.pos)) {
    u32 hash = guard_data_hash(&gd);
    if (!aoc_set_guard_data_contains_pre_hashed(loop_set, gd, hash)) {
      aoc_set_guard_data_insert_pre_hashed(loop_set, gd, hash);
    } else {
      return true;
    }

    move(input, &gd);
  }

  return false;
}

static void solve(char *const input, const point start,
                  aoc_set_point *const visited, aoc_set_point *const blocks,
                  aoc_set_guard_data *const loop_set, u32 *const part1,
                  u32 *const part2) {
  guard_data gd = {start, DIRECTION_UP};
  guard_data prev = gd;

  // for skipping the starting point for part 2
  aoc_set_point_insert(visited, gd.pos);
  move(input, &gd);

  while (BOUNDS_CHECK(gd.pos)) {
    u32 hash = aoc_point2_u8_hash(&gd.pos);
    if (!aoc_set_point_contains_pre_hashed(visited, gd.pos, hash)) {
      aoc_set_point_insert_pre_hashed(visited, gd.pos, hash);

      const size_t i = gd.pos.y * INPUT_WIDTH_NL + gd.pos.x;
      input[i] = '#';
      if (!aoc_set_point_contains(blocks, gd.pos) &&
          check_loop(input, prev.pos, prev.dir, loop_set)) {
        aoc_set_point_insert(blocks, gd.pos);
      }
      input[i] = '.';
    }

    prev = gd;
    move(input, &gd);
  }

  *part1 = visited->count;
  *part2 = blocks->count;
}

int main(void) {
  aoc_set_point visited = {0};
  aoc_set_point blocks = {0};
  aoc_set_guard_data loop_set = {0};
  aoc_set_point_create(&visited, 1 << 13);
  aoc_set_point_create(&blocks, 1 << 11);
  aoc_set_guard_data_create(&loop_set, 1 << 11);

  char *input = aoc_file_read_all2("day06/input.txt");
  const point start = get_start_point(input);

  u32 part1, part2;
  solve(input, start, &visited, &blocks, &loop_set, &part1, &part2);

  aoc_free(input);
  aoc_set_point_destroy(&visited);
  aoc_set_point_destroy(&blocks);
  aoc_set_guard_data_destroy(&loop_set);

  printf("%u %u\n", part1, part2);
}
