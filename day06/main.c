#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <aoc/filesystem.h>
#include <stdio.h>
#include <threads.h>

// injected from the Makefile
#ifndef INPUT_HEIGHT
#define INPUT_HEIGHT 1
#endif

#ifndef INPUT_WIDTH
#define INPUT_WIDTH 1
#endif

#ifndef CPU_CORES
#define CPU_CORES 1
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

static inline void move(const char *const input, const point block,
                        guard_data *const gd) {
  point new_pos = {
      gd->pos.x + move_offsets[gd->dir].x,
      gd->pos.y + move_offsets[gd->dir].y,
  };

  size_t i = new_pos.y * INPUT_WIDTH_NL + new_pos.x;

  while (BOUNDS_CHECK(new_pos) &&
         (input[i] == '#' || (new_pos.x == block.x && new_pos.y == block.y))) {
    gd->dir = (gd->dir + 1) & 3;
    new_pos.x = gd->pos.x + move_offsets[gd->dir].x;
    new_pos.y = gd->pos.y + move_offsets[gd->dir].y;
    i = new_pos.y * INPUT_WIDTH_NL + new_pos.x;
  }

  gd->pos = new_pos;
}

static bool check_loop(const char *const input, const point start,
                       const direction start_dir, const point block,
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

    move(input, block, &gd);
  }

  return false;
}

typedef struct move_data {
  guard_data from;
  guard_data to;
} move_data;

#define AOC_T move_data
#include <aoc/vector.h>

typedef struct thread_data {
  aoc_set_guard_data loop_set;
  aoc_set_point *blocks;
  const aoc_vector_move_data *moves;
  size_t from;
  size_t to;
  mtx_t *mtx;
  const char *input;
} thread_data;

static u32 solve_part1(char *const input, const point start,
                       aoc_set_point *const visited,
                       aoc_vector_move_data *const moves) {
  guard_data gd = {start, DIRECTION_UP};
  guard_data prev = gd;

  // for skipping the starting point for part 2
  aoc_set_point_insert(visited, gd.pos);
  move(input, (point){0xff, 0xff}, &gd);

  while (BOUNDS_CHECK(gd.pos)) {
    u32 hash = aoc_point2_u8_hash(&gd.pos);
    if (!aoc_set_point_contains_pre_hashed(visited, gd.pos, hash)) {
      aoc_set_point_insert_pre_hashed(visited, gd.pos, hash);

      // collecting for part 2
      aoc_vector_move_data_push(moves, (move_data){prev, gd});
    }

    prev = gd;
    move(input, (point){0xff, 0xff}, &gd);
  }

  return visited->count;
}

static i32 run_part2(thread_data *data) {
  for (size_t i = data->from; i < data->to; ++i) {
    move_data *const m = &data->moves->items[i];

    mtx_lock(data->mtx);
    if (!aoc_set_point_contains(data->blocks, m->to.pos)) {
      mtx_unlock(data->mtx);

      if (check_loop(data->input, m->from.pos, m->from.dir, m->to.pos,
                     &data->loop_set)) {
        mtx_lock(data->mtx);
        aoc_set_point_insert(data->blocks, m->to.pos);
        mtx_unlock(data->mtx);
      }
    } else {
      mtx_unlock(data->mtx);
    }
  }

  return 0;
}

static u32 solve_part2(const char *const input, aoc_set_point *const blocks,
                       const aoc_vector_move_data *const moves) {
  mtx_t mutex = {0};
  mtx_init(&mutex, mtx_plain);

  const size_t stride = moves->length / CPU_CORES;
  thread_data data[CPU_CORES] = {0};
  for (size_t i = 0; i < CPU_CORES; ++i) {
    data[i].from = i * stride;
    // due to rounding errors the last core will have slightly more work to do
    data[i].to = i == (CPU_CORES - 1) ? moves->length : (i + 1) * stride;
    data[i].mtx = &mutex;
    data[i].moves = moves;
    data[i].blocks = blocks;
    data[i].input = input;
    aoc_set_guard_data_create(&data[i].loop_set, 1 << 11);
  }

  thrd_t threads[CPU_CORES] = {0};
  for (size_t i = 0; i < CPU_CORES; ++i) {
    thrd_create(&threads[i], (thrd_start_t)run_part2, &data[i]);
  }

  for (size_t i = 0; i < CPU_CORES; ++i) {
    int result = 0;
    thrd_join(threads[i], &result);
    aoc_set_guard_data_destroy(&data[i].loop_set);
  }

  mtx_destroy(&mutex);
  return blocks->count;
}

int main(void) {
  aoc_set_point visited = {0};
  aoc_set_point blocks = {0};
  aoc_set_point_create(&visited, 1 << 13);
  aoc_set_point_create(&blocks, 1 << 11);

  char *input = aoc_file_read_all2("day06/input.txt");
  const point start = get_start_point(input);

  aoc_vector_move_data moves = {0};
  aoc_vector_move_data_create(&moves, 1 << 11);

  const u32 part1 = solve_part1(input, start, &visited, &moves);
  const u32 part2 = solve_part2(input, &blocks, &moves);

  aoc_free(input);
  aoc_vector_move_data_destroy(&moves);
  aoc_set_point_destroy(&visited);
  aoc_set_point_destroy(&blocks);

  printf("%u %u\n", part1, part2);
}
