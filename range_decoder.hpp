#include <cstdint>
#include <stdio.h>
#include <array>
#include <algorithm>

// see https://en.wikipedia.org/wiki/Range_encoding

template <class Context, class InIter, class OutIter>
OutIter
range_decoder(Context &ctxt, OutIter dest, OutIter destmax, InIter begin, InIter end) {
  constexpr int shift = 64 - 8;
  typedef uint64_t acc_t;
  acc_t low = 0;
  acc_t range = ~(uint64_t)0;

  acc_t mask = ctxt.mask;
  auto p = begin;
  acc_t code = 0;
  for (int i = 0; i != sizeof(acc_t); ++i) {
    if (p == end) return destmax;
    code = code * 0x100 + (*p++ & 0xff);
  }

  constexpr size_t total = 0x10000;
  printf("size=%ld total=%ld\n", ctxt.size, total);

  auto &symbols = *new std::array<uint8_t, 65536>{};
  for (int i = 0, sym = 0; sym != mask+1; ++sym) {
    acc_t size = ctxt.starts[sym+1] - ctxt.starts[sym];
    for (acc_t j = 0; j != size; ++j) {
      symbols[i++] = sym;
    }
  }

  size_t max_size = std::min(ctxt.size, size_t(destmax - dest));
  for (int i = 0; i != max_size; ++i) {
    acc_t divisor = range / total;
    acc_t value = (code - low) / divisor;
    uint8_t symbol = symbols[value];
    uint32_t start = ctxt.starts[symbol];
    uint32_t size = ctxt.starts[symbol+1] - ctxt.starts[symbol];

    range = divisor;
    low += start * range;
    range *= size;

    //printf("%02x [%04x..%04x] range=%016lx..%016lx [%016lx]\n", symbol, start, start+size, long(low), long(low+range), long(range));
    
    *dest++ = symbol;

    // if the top byte is the same, output the byte and increase the range
    while ((low >> shift) == ((low + range) >> shift)) {
      if (p == end) { ctxt.error(i, "input overrun 1"); goto finish; }
      code = code * 0x100 + (*p++ & 0xff);
      low <<= 8;
      range <<= 8;
    }

    // if the range is too small, output some bytes and increase the range.
    if (range < 0x10000) {
      if (p == end) { ctxt.error(i, "input overrun 1"); goto finish; }
      code = code * 0x100 + (*p++ & 0xff);
      if (p == end) { ctxt.error(i, "input overrun 1"); goto finish; }
      code = code * 0x100 + (*p++ & 0xff);
      low <<= 16;
      range = ~low;
    }
  }
finish:

  delete &symbols;
  return dest;
}

