# Bit Counting Benchmark
## License

Everything in this project is authored by me (Joe Zbiciak,
joe.zbiciak@leftturnonly.info), and is licensed under the Creative Commons
Attribution-ShareAlike 4.0 International license, aka.
[CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).

## Background

This is a really simple benchmark of different methods for counting 1 bits
in a 32-bit value.


## Description

The benchmark itself uses the IEEE CRC-32 to generate a sequence of random
32-bit values.  The IEEE CRC-32 polynomial is an maximal sequence polynomial
for an LFSR.

The benchmark first checks that the three computational methods (`popcnt32_a`,
`popcnt32_b`, and `popcnt32_c`) all return the same values for the full 32-bit
input range.  This takes a few seconds on a modern computer.

Next, it initializes a lookup table for a fourth version, `popcnt32_d`, that
merely looks up the value in the lookup table.

Finally, it measures the amount of time it takes to run through the entire
2^32 - 1 sequence of the IEEE CRC-32, counting the 1 bits in each state. In
order to force the compiler to measure this, the code uses a `volatile` sum
variable to sum up the 1 bits counted.

Because computing the CRC sequence also takes time, the code also measures that
as the "Null" loop.  You can see from the run time that the Null loop takes
nearly as long as `popcnt32_a`, `popcnt32_b`, and `popcnt32_c` on many systems.
Meanwhile, `popcnt32_d` takes a lot longer than the others

## Data

The following output comes from my M1 Max based MacBook Pro, compiled with
`clang -mtune=native -O3 -o bit_count bit_count.c` with Apple clang version
13.1.6 (clang-1316.0.21.2.3).

```
$ ./bit_count
Testing implementations...
Errs: 0  OK: 4294967296
Initializing LUT implementation... Done.
Null:           6055631 clocks
Ver A:          8167488 clocks
Ver B:          8268174 clocks
Ver C:          7897421 clocks
Ver D:         34834314 clocks
Sums: 1000000000 1000000000 1000000000 1000000000
Null sum: 7FFFFFFF80000000
$
```

____

Copyright © 2023, Joe Zbiciak <joe.zbiciak@leftturnonly.info>  
`SPDX-License-Identifier:  CC-BY-SA-4.0`
