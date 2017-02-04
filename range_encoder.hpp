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

  printf("start\n");
  for (auto p = begin; p != end; ++p) {
    ctxt.sizes[*p & mask]++;
  }

  printf("calculated sizes\n");
  return dest;

  ctxt.total = uint32_t(end - begin);
  for (uint32_t i = 0, start = 0; i != mask+1; ++i) {
    ctxt.starts[i] = start;
    start += ctxt.sizes[i];
  }

  uint32_t low = 0;
  uint32_t range = 0xffffffff;
  for (auto p = begin; p != end; ++p) {
    auto start = ctxt.starts[*p & mask];
    auto size = ctxt.sizes[*p & mask];

    range /= ctxt.total;
    low += start * range;
    range *= size;

    //printf("%02x [%04x..%04x] range=%08x..%08x\n", *p & mask, start, start+size, low, low+range);

    // if the top byte is the same output the byte and increase the range
    while ((low >> 24) == ((low + range) >> 24)) {
      *dest++ = uint8_t(low >> 24);
      if (dest >= destmax) return dest;
      low <<= 8;
      range <<= 8;
    }

    // if the range is too small, output some bytes and increase the range.
    if (range < 0x10000) {
      *dest++ = uint8_t(low >> 24);
      if (dest >= destmax) return dest;
      low <<= 8;
      *dest++ = uint8_t(low >> 24);
      if (dest >= destmax) return dest;
      low <<= 8;
      range = 0xffffffff - low;
    }
  }

  while (range < 0x10000) {
    //printf("emit %02x\n", low >> 24);
    *dest++ = uint8_t(low >> 24);
    if (dest >= destmax) return dest;
    low <<= 8;
    range <<= 8;
  }

  low += 0x1000000;
  *dest++ = uint8_t(low >> 24);

  return dest;
}

