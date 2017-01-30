#include <cstdint>
#include <stdio.h>
#include <array>

// see https://en.wikipedia.org/wiki/Range_encoding

template <class Context, class InIter, class OutIter>
OutIter
range_decoder(Context &ctxt, OutIter dest, InIter begin, InIter end) {
  uint32_t low = 0;
  uint32_t range = 0xffffffff;
  constexpr int shift = 24;
  constexpr int shift2 = 16;

  uint32_t mask = ctxt.mask;
  auto p = begin;
  uint32_t code = 0;
  for (int i = 0; i != 4; ++i) {
    code = code * 0x100 + (*p++ & 0xff);
  }

  for (int i = 0; i != ctxt.total; ++i) {
    uint32_t divisor = range / ctxt.total;
    uint32_t value = (code - low) / divisor;
    auto lb = std::upper_bound(ctxt.starts.begin(), ctxt.starts.end(), value);

    uint32_t symbol = lb - 1 - ctxt.starts.begin();
    uint32_t start = lb[-1];
    uint32_t size = ctxt.sizes[symbol];

    range = divisor;
    low += start * range;
    range *= size;

    printf("%02x [%04x..%04x] range=%08x..%08x\n", symbol, start, start+size, low, low+range);

    if (range < 0x10000) {
      code = code * 0x100 + (*p++ & 0xff);
      code = code * 0x100 + (*p++ & 0xff);
      low <<= 16;
      range = 0xffffffff - low;
    }

    while ((low >> shift) == ((low + range) >> shift)) {
      code = code * 0x100 + (*p++ & 0xff);
      low <<= 8;
      range <<= 8;
    }
  }
  return dest;
}

