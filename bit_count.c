// This software is licensed under Creative Commons Attribution-ShareAlike 4.0
// International (CC BY-SA 4.0)
// 
// Copyright 2023, Joe Zbiciak <joe.zbiciak@leftturnonly.info>
// SPDX-License-Identifier:  CC-BY-SA-4.0

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

enum {
  kMu0 = 0x55555555ul,
  kMu1 = 0x33333333ul,
  kMu2 = 0x0F0F0F0Ful,
  kMu3 = 0x00FF00FFul,
  kMu4 = 0x0000FFFFul
};

static int popcnt32_a(uint32_t x) {
  x = (x & kMu0) + ((x >>  1) & kMu0);
  x = (x & kMu1) + ((x >>  2) & kMu1);
  x = (x & kMu2) + ((x >>  4) & kMu2);
  x = (x & kMu3) + ((x >>  8) & kMu3);
  x = (x & kMu4) + ((x >> 16) & kMu4);
  return (int)x;
}

static int popcnt32_b(uint32_t x) {
  x = (x & kMu0) + ((x >> 1) & kMu0);
  x = (x & kMu1) + ((x >> 2) & kMu1);
  x = (x + (x >> 4)) & kMu2;
  x += x >> 8;
  x += x >> 16;
  return (uint8_t)x;
}

static int popcnt32_c(uint32_t x) {
  x = (x & kMu0) + ((x >> 1) & kMu0);
  x = (x & kMu1) + ((x >> 2) & kMu1);
  x = (x + (x >> 4)) & kMu2;
  x = (x * 0x01010101) >> 24;
  return (int)x;
}

#ifdef HAVE_BUILTIN_POPCOUNT
static int popcnt32_d(uint32_t x) {
  return __builtin_popcount(x);
}
#endif

static uint8_t *popcnt32_lut;

static void init_popcnt32_lut(void) {
  popcnt32_lut = realloc(popcnt32_lut, UINT32_MAX + 1ull);
  if (!popcnt32_lut) {
    fprintf(stderr, "OOM.");
    exit(1);
  }

  for (uint64_t i = 0; i <= UINT32_MAX; ++i) {
    popcnt32_lut[i] = popcnt32_a(i);
  }
}

static int popcnt32_z(uint32_t x) {
  return popcnt32_lut[x];
}

#define CRC32_POLY (0xEDB88320)             // CRC-32 (IEEE)

#define popcnt32_null(x) (x)

#define BENCH(name,func)                                                      \
    do {                                                                      \
      volatile uint64_t sum = 0;                                              \
      t1 = clock();                                                           \
      do {                                                                    \
        sum += func(r);                                                       \
        r = (r >> 1) ^ (-(r & 1) & CRC32_POLY);                               \
        sum += func(r);                                                       \
        r = (r >> 1) ^ (-(r & 1) & CRC32_POLY);                               \
        sum += func(r);                                                       \
        r = (r >> 1) ^ (-(r & 1) & CRC32_POLY);                               \
        sum += func(r);                                                       \
        r = (r >> 1) ^ (-(r & 1) & CRC32_POLY);                               \
        sum += func(r);                                                       \
        r = (r >> 1) ^ (-(r & 1) & CRC32_POLY);                               \
      } while (r != 1);                                                       \
      t2 = clock();                                                           \
      printf("%6s:   %15lld clocks  %"PRIX64"\n", name,                       \
             (unsigned long long)t2 - t1, sum);                               \
    } while (0)

int main() {
  int64_t errs = 0, ok = 0;

  printf("Testing implementations...\n");

  for (uint64_t i = 0; i <= UINT32_MAX; ++i) {
    const int a = popcnt32_a(i);
    const int b = popcnt32_b(i);
    const int c = popcnt32_c(i);
#ifdef HAVE_BUILTIN_POPCOUNT
    const int d = popcnt32_d(i);
#else
    const int d = c;
#endif

    if (a != b || a != c || a != d) {
      fprintf(stderr, "%08"PRIX64": %d %d %d %d\n", i, a, b, c, d);
      errs++;
      if (errs > 10) {
        break;
      }
    } else {
      ok++;
    }
  }

  printf("Errs: %"PRId64"  OK: %"PRId64"\n", errs, ok);

  printf("Initializing LUT implementation..."); fflush(stdout);
  init_popcnt32_lut();
  puts(" Done.");

  clock_t t1, t2;
  uint32_t r = 1;

  BENCH("Null",  popcnt32_null);
  BENCH("Ver A", popcnt32_a);
  BENCH("Ver B", popcnt32_b);
  BENCH("Ver C", popcnt32_c);
#ifdef HAVE_BUILTIN_POPCOUNT
  BENCH("Ver D", popcnt32_d);
#endif
  BENCH("Ver Z", popcnt32_z);
}
