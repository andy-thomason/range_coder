#include <cstdint>
#include <stdio.h>
#include <array>

// see https://en.wikipedia.org/wiki/Range_encoding

template <class InIter, class OutIter, uint32_t SymBits=8>
OutIter
range_coder(OutIter dest, InIter begin, InIter end) {
  constexpr uint32_t mask = ((uint32_t)1 << SymBits) - 1;
  std::array<uint32_t, mask+1> starts;
  std::array<uint32_t, mask+1> sizes;

  for (auto &p : sizes) {
    p = 0;
  }

  for (auto p = begin; p != end; ++p) {
    sizes[*p & mask]++;
  }

  auto total = uint32_t(end - begin);
  for (uint32_t i = 0, start = 0; i != mask+1; ++i) {
    starts[i] = start;
    start += sizes[i];
  }

  uint32_t low = 0;
  uint32_t range = 0xffffffff;
  constexpr int shift = 24;
  for (auto p = begin; p != end; ++p) {
    auto start = starts[*p & mask];
    auto size = sizes[*p & mask];

    range /= total;
    low += start * range;
    range *= size;

    printf("%02x [%04x..%04x] range=%08x..%08x\n", *p & mask, start, start+size, low, low+range);

    if (range < 0x10000) {
      *dest++ = low >> shift;
      low <<= 8;
      range <<= 8;
      *dest++ = low >> shift;
      low <<= 8;
      range <<= 8;
      range = 0xffffffff - low;
    }

    while ((low >> shift) == ((low + range) >> shift)) {
      printf("emit %02x\n", low >> shift);
      *dest++ = low >> shift;
      low <<= 8;
      range <<= 8;
    }
  }

  while (range < 0x1000) {
    *dest++ = low >> shift;
    low <<= 8;
    range <<= 8;
  }

  low += 0x1000000;
  *dest++ = low >> shift;

  return dest;
}

