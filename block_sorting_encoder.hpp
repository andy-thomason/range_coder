#include "suffix_array.hpp"

#include <cstdint>
#include <stdio.h>
#include <array>
#include <algorithm>

template <class Context, class InIter, class OutIter, uint32_t SymBits=8>
OutIter
block_sorting_encoder(Context &ctxt, OutIter dest, OutIter destmax, InIter begin, InIter end) {
  constexpr size_t block_size = 0x100000;

  for (auto start = begin; start < end; start += block_size) {
    auto last = std::min(end, start + block_size);

    suffix_array<uint8_t> sa(start, last);
    size_t size = sa.size();
    for (size_t i = 0; i != size && dest != destmax; ++i) {
      auto addr = sa.addr(i);
      *dest++ = addr == 0 ? '$' : start[addr-1];
    }
  }

  return dest;
}

