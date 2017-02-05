#include <cstdint>
#include <stdio.h>
#include <array>
#include <algorithm>

// see https://en.wikipedia.org/wiki/Range_encoding

template <class Context, class InIter, class OutIter, uint32_t SymBits=8>
OutIter
range_encoder(Context &ctxt, OutIter dest, OutIter destmax, InIter begin, InIter end) {
  constexpr uint32_t mask = ctxt.mask;

  printf("start\n");
  std::array<size_t, mask+1> sizes;
  
  std::fill(sizes.begin(), sizes.end(), 0);

  for (auto p = begin; p != end; ++p) {
    sizes[*p & mask]++;
  }

  size_t size = size_t(end - begin);
  size_t total = size;
  if (size > 0xffff-mask) {
    size_t divisor = size / (0xffff-mask);
    total = 0;
    for (int i = 0; i != mask+1; ++i) {
      size_t size = sizes[i] / divisor;
      sizes[i] = size != 0 ? size : 1;
      total += sizes[i];
    }
  }

  printf("calculated size=%ld / total=%ld\n", long(size), long(total));

  ctxt.total = total;
  ctxt.size = size;
  for (uint32_t i = 0, start = 0; i != mask+1; ++i) {
    ctxt.starts[i] = start;
    start += sizes[i];
  }
  ctxt.starts[mask+1] = total;

  uint32_t low = 0;
  uint32_t range = 0xffffffff;
  for (auto p = begin; p != end; ++p) {
    /*if (((p-begin) & 0xffffff) == 0) {
      printf("%lx / %lx\n", long(p-begin)>>24, long(end-begin)>>24);
    }*/
    auto start = ctxt.starts[*p & mask];
    auto size = sizes[*p & mask];

    range /= ctxt.total;
    low += start * range;
    range *= size;

    //printf("%02x [%04lx..%04lx] range=%08x..%08x\n", *p & mask, long(start), long(start+size), low, low+range);

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

