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
  return (int)(x & 63);
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

#define CRC64_POLY (0xC96C5795D7870F42ull)  // CRC-64-ECMA
#define CRC32_POLY (0xEDB88320)             // CRC-32 (IEEE)

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
  volatile uint64_t sn = 0, sa = 0, sb = 0, sc = 0, sd = 0, sz = 0;

  t1 = clock();
  r = 1;
  sn = 0;
  do {
    sn += r;
    r = (r >> 1) ^ (r & 1 ? CRC32_POLY : 0);
  } while (r != 1);
  t2 = clock();

  printf("Null:   %15lld clocks\n", (unsigned long long)t2 - t1);

  t1 = clock();
  r = 1;
  sa = 0;
  do {
    sa += popcnt32_a(r);
    r = (r >> 1) ^ (r & 1 ? CRC32_POLY : 0);
  } while (r != 1);
  t2 = clock();

  printf("Ver A:  %15lld clocks\n", (unsigned long long)t2 - t1);

  t1 = clock();
  r = 1;
  sb = 0;
  do {
    sb += popcnt32_b(r);
    r = (r >> 1) ^ (r & 1 ? CRC32_POLY : 0);
  } while (r != 1);
  t2 = clock();

  printf("Ver B:  %15lld clocks\n", (unsigned long long)t2 - t1);

  t1 = clock();
  r = 1;
  sc = 0;
  do {
    sc += popcnt32_c(r);
    r = (r >> 1) ^ (r & 1 ? CRC32_POLY : 0);
  } while (r != 1);
  t2 = clock();

  printf("Ver C:  %15lld clocks\n", (unsigned long long)t2 - t1);

#ifdef HAVE_BUILTIN_POPCOUNT
  t1 = clock();
  r = 1;
  sd = 0;
  do {
    sd += popcnt32_d(r);
    r = (r >> 1) ^ (r & 1 ? CRC32_POLY : 0);
  } while (r != 1);
  t2 = clock();

  printf("Ver D:  %15lld clocks\n", (unsigned long long)t2 - t1);
#else
  sd = sa;
#endif

  t1 = clock();
  r = 1;
  sz = 0;
  do {
    sz += popcnt32_z(r);
    r = (r >> 1) ^ (r & 1 ? CRC32_POLY : 0);
  } while (r != 1);
  t2 = clock();

  printf("Ver Z:  %15lld clocks\n", (unsigned long long)t2 - t1);

  printf("Sums: %"PRIX64" %"PRIX64" %"PRIX64" %"PRIX64" %"PRIX64"\n",
         sa, sb, sc, sd, sz);

  printf("Null sum: %"PRIX64"\n", sn);
}
