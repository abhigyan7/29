#ifndef BITUTILS_H_
#define BITUTILS_H_

#include <stdint.h>
namespace bitutils {

#define LOG2(X) ((unsigned)(8 * sizeof(uint32_t) - __builtin_clz((X)) - 1))

uint32_t set_nth_bit_to(uint32_t integer, int n, int val);
inline uint32_t set_nth_bit_to(uint32_t integer, int n, int val) {
  integer ^= (-val ^ integer) & (1ULL << n);
  return integer;
}

int popcount(uint32_t in);
inline int popcount(uint32_t in) {
  int ret;

  for (ret = 0; in; ret++) {
    in &= in - 1; // clear the least significant bit set
  }
  return ret;
}

inline int get_nth_bit(uint32_t integer, int n) { return (integer >> n) & 1U; }

int pop_next_index(uint32_t &in);
inline int pop_next_index(uint32_t &in) {
  if (!in) {
    return -1;
  }
  int ret = LOG2(in);
  in = set_nth_bit_to(in, ret, 0);
  return ret;
}

inline uint8_t n_trailing_bits(uint32_t num) {

  unsigned int v; // input to count trailing zero bits
  uint8_t c;      // output: c will count v's trailing zero bits,
                  // so if v is 1101000 (base 2), then c will be 3
  if (num) {
    num = (num ^ (num - 1)) >> 1; // Set v's trailing 0s to 1s and zero rest
    for (c = 0; num; c++) {
      num >>= 1;
    }
  } else {
    c = 32;
  }
  return c;
}
} // namespace bitutils
#endif // BITUTILS_H_
