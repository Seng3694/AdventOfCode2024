#include <aoc/aoc.h>
#include <aoc/bits.h>
#include <aoc/common.h>
#include <aoc/filesystem.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

// awk '{print NF - 1}' day07/input.txt | sort -un | tail -n1
// max amount of operands -> 12
#define MAX_OPERAND_COUNT 12
#define MAX_OPERATOR_COUNT (MAX_OPERAND_COUNT - 1)

// sed -re 's/^[0-9]+: //g' day07/input.txt | grep -Po '\d+' | sort -un | tail
// -n 1 biggest operand -> 999

// sed -re 's/^[0-9]+: //g' day07/input.txt | grep -Po '\d+' | sort -un | head
// -n 1 smallest operand -> 1

// grep -Po '^\d+(?=:)' day07/input.txt | sort -un | tail -n 1
// biggest test value -> 95914368903131

// grep -Po '^\d+(?=:)' day07/input.txt | sort -un | head -n 1
// smallest test value -> 43

// can use u16 for operands and u64 for test value

typedef struct equation {
  u64 test_value;
  u16 operands[MAX_OPERAND_COUNT];
  u8 operand_count;
} equation;

typedef struct context {
  equation equations[INPUT_HEIGHT];
  u32 result;
  u16 count;
} context;

typedef enum operator{
  OPERATOR_ADD,
  OPERATOR_MULTIPLY,
  OPERATOR_CONCATENATE,
} operator;

// this only works for b values smaller than 1000
static inline u64 operation_concat(const u64 a, const u64 b) {
  return a * ((b < 10) ? 10 : (b < 100) ? 100 : 1000) + b;
}

static void parse(char *line, context *const ctx) {
  // test_value: operand1 operand2 ... operandN
  equation *const eq = &ctx->equations[ctx->count++];
  eq->test_value = strtoull(line, &line, 10);
  // skip colon and space
  line += 2;
  while (*line) {
    eq->operands[eq->operand_count++] = strtoul(line, &line, 10);
    // skip space or newline. now at either new number or \0
    ++line;
  }
}

static void solve(const context *const ctx, u64 *const part1,
                  u64 *const part2) {
  u64 p1 = 0;
  u64 p2 = 0;
  for (u16 i = 0; i < ctx->count; ++i) {
    const equation *const eq = &ctx->equations[i];
    u32 upper = powf(2, eq->operand_count - 1);

    // part1
    for (u32 c = 0; c < upper; ++c) {
      u64 result = eq->operands[0];

      for (size_t i = 0; i < eq->operand_count - 1; ++i) {
        const operator op = AOC_CHECK_BIT(c, i);
        result = op == OPERATOR_ADD ? result + eq->operands[i + 1]
                                    : result * eq->operands[i + 1];
      }

      if (result == eq->test_value) {
        p1 += eq->test_value;
        p2 += eq->test_value;
        goto next_equation;
      }
    }

    // part2
    upper = powf(3, eq->operand_count - 1);

    for (u32 c = 0; c < upper; ++c) {
      u64 result = eq->operands[0];
      u32 temp = c;

      // this goes through the combinations with only + and * again
      // should optimize it to only use combinations including at least one ||
      for (size_t i = 0; i < eq->operand_count - 1; ++i) {
        const operator op = temp % 3;

        switch (op) {
        case OPERATOR_ADD:
          result += eq->operands[i + 1];
          break;
        case OPERATOR_MULTIPLY:
          result *= eq->operands[i + 1];
          break;
        case OPERATOR_CONCATENATE:
          result = operation_concat(result, eq->operands[i + 1]);
          break;
        }
        temp /= 3;
      }

      if (result == eq->test_value) {
        p2 += eq->test_value;
        goto next_equation;
      }
    }

  next_equation:;
  }

  *part1 = p1;
  *part2 = p2;
}

int main(void) {
  context ctx = {0};
  aoc_file_read_lines1("day07/input.txt", (aoc_line_func)parse, &ctx);

  u64 part1 = 0, part2 = 0;
  solve(&ctx, &part1, &part2);

  printf("%lu %lu\n", part1, part2);
}
