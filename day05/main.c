#include <aoc/aoc.h>
#include <aoc/filesystem.h>
#include <stdio.h>
#include <stdlib.h>

// grep -Po '(?<=\|)(\d+)' day05/input.txt | sort | uniq -c | sort -nrk1 | head
// -n 1 | awk '{print $1}' one page does not depend on more than 24 pages they
// actually all have exactly 24 pages but I will still parameterize it for the
// example
#define MAX_DEPENDENCY_COUNT 24

// grep -Po '\d+' day05/input.txt | sort -un | tail -n 1
// grep -Po '\d+' day05/input.txt | sort -unr | tail -n 1
// pageIds are within the range of 0 and 100. fits in u8
// this also means that I can use an array with 100 elements for the pages
// instead of a map
#define MAX_PAGE_COUNT 100

// sed -re 's/\||,/ /g' day05/input.txt | awk '{print NF}' | sort -un | tail -n
// maximum amount of pages per update are 23. going with 24 instead
#define MAX_UPDATE_PAGE_COUNT 24

// grep -P ',' day05/input.txt | wc -l
// there are only 202 updates. going with 255
#define MAX_UPDATE_COUNT 255

typedef struct page {
  u8 dependencies[MAX_DEPENDENCY_COUNT];
  u8 count;
} page;

typedef struct update {
  u8 pages[MAX_UPDATE_PAGE_COUNT];
  u8 count;
} update;

static page pages[MAX_PAGE_COUNT] = {0};
static update updates[MAX_UPDATE_COUNT] = {0};
static u8 updateCount = 0;

static void parse_line(char *line, void *userData) {
  (void)userData;
  // grep -Po '\d+' day05/input.txt | sort -u | awk '{print length}' | sort -unr
  // every number has exactly 2 digits

  // line has one of the following formats:
  // "\d{2}|\d{2}" or "(\d{2},)+\d{2}" or new line

  if (line[0] == '\n')
    return;

  if (line[2] == '|') {
    const u8 left = strtoul(line, &line, 10);
    const u8 right = strtoul(line + 1, &line, 10);
    AOC_ASSERT(left < MAX_PAGE_COUNT);
    AOC_ASSERT(right < MAX_PAGE_COUNT);
    AOC_ASSERT(pages[right].count < MAX_DEPENDENCY_COUNT);
    pages[right].dependencies[pages[right].count++] = left;
  } else /* ',' */ {
    AOC_ASSERT(updateCount < MAX_UPDATE_COUNT);
    update *const u = &updates[updateCount];
    while (*line) {
      AOC_ASSERT(u->count + 1 < MAX_UPDATE_PAGE_COUNT);
      const u8 n = strtoul(line, &line, 10);
      AOC_ASSERT(n < MAX_PAGE_COUNT);
      u->pages[u->count++] = n;
      // this either skips the comma or the newline
      // ending up at the next number or null terminator
      ++line;
    }
    ++updateCount;
  }
}

static i32 compare(const u8 *const left, const u8 *const right) {
  for (u8 d = 0; d < pages[*left].count; ++d) {
    if (pages[*left].dependencies[d] == *right) {
      return -1;
    }
  }
  return 1;
}

static void solve(u32 *const part1, u32 *const part2) {
  u32 s1 = 0;
  u32 s2 = 0;

  for (u8 u = 0; u < updateCount; ++u) {
    // go through all ids
    // for each id check if none of the following ids are dependencies
    for (u8 p1 = 0; p1 < updates[u].count - 1; ++p1) {
      const u8 pid1 = updates[u].pages[p1];
      for (u8 p2 = p1 + 1; p2 < updates[u].count; ++p2) {
        const u8 pid2 = updates[u].pages[p2];
        for (u8 d = 0; d < pages[pid1].count; ++d) {
          if (pages[pid1].dependencies[d] == pid2) {
            // fix order for part 2
            qsort(updates[u].pages, updates[u].count, sizeof(u8),
                  (__compar_fn_t)compare);
            s2 += updates[u].pages[updates[u].count / 2];
            goto next;
          }
        }
      }
    }

    s1 += updates[u].pages[updates[u].count / 2];

  next:;
  }

  *part1 = s1;
  *part2 = s2;
}

int main(void) {
  aoc_file_read_lines1("day05/input.txt", (aoc_line_func)parse_line, NULL);
  u32 part1, part2;
  solve(&part1, &part2);
  printf("%u %u\n", part1, part2);
}
