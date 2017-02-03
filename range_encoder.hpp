#include <cstdint>
#include <stdio.h>
#include <array>

// see https://en.wikipedia.org/wiki/Range_encoding

template <class Context, class InIter, class OutIter, uint32_t SymBits=8>
OutIter
range_encoder(Context &ctxt, OutIter dest, OutIter destmax, InIter begin, InIter end) {
  constexpr uint32_t mask = ctxt.mask;

  for (auto &p : ctxt.sizes) {
    p = 0;
  }

  for (auto p = begin; p != end; ++p) {
    ctxt.sizes[*p & mask]++;
  }

  ctxt.total = uint32_t(end - begin);
  for (uint32_t i = 0, start = 0; i != mask+1; ++i) {
    ctxt.starts[i] = start;
    start += ctxt.sizes[i];
  }

  uint32_t low = 0;
  uint32_t range = 0xffffffff;
  constexpr int shift = 24;
  constexpr int shift2 = 16;
  for (auto p = begin; p != end; ++p) {
    auto start = ctxt.starts[*p & mask];
    auto size = ctxt.sizes[*p & mask];

    range /= ctxt.total;
    low += start * range;
    range *= size;

    printf("%02x[%04x..%04x] range=%08x..%08x\n", *p & mask, start, start+size, low, low+range);

    while ((low >> shift) == ((low + range) >> shift)) {
      printf("emit %02x\n", low >> shift);
      *dest++ = uint8_t(low >> shift);
      if (dest >= destmax) return dest;
      low <<= 8;
      range <<= 8;
    }

    if (range < 0x10000) {
      printf("emit2 %02x\n", low >> shift);
      *dest++ = uint8_t(low >> shift);
      if (dest >= destmax) return dest;
      low >>= 8;
      printf("emit2 %02x\n", low >> shift);
      *dest++ = uint8_t(low >> shift);
      if (dest >= destmax) return dest;
      low >>= 8;
      range = 0xffffffff - low;
    }
  }

  while (range < 0x10000) {
    printf("emit %02x\n", low >> shift);
    *dest++ = uint8_t(low >> shift);
    if (dest >= destmax) return dest;
    low <<= 8;
    range <<= 8;
  }

  low += 0x1000000;
  *dest++ = uint8_t(low >> shift);

  return dest;
}

