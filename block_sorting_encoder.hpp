#include "suffix_array.hpp"

#include <cstdint>
#include <stdio.h>
#include <array>
#include <algorithm>
#include <numeric>

template <class Context, class InIter, class OutIter, uint32_t SymBits=8>
OutIter
block_sorting_encoder(Context &ctxt, OutIter dest, OutIter destmax, InIter begin, InIter end) {
  constexpr size_t block_size = 1024 * 900;

  for (auto start = begin; start < end; start += block_size) {
    auto last = std::min(end, start + block_size);

    suffix_array<uint8_t, uint32_t> sa(start, last);
    size_t size = sa.size();
    std::array<uint8_t, 256> mtf;
    std::iota(mtf.begin(), mtf.end(), 0);
    std::array<uint8_t, 256> rank;
    std::iota(rank.begin(), rank.end(), 0);

    for (size_t i = 0; i != size && dest != destmax; ++i) {
      auto addr = sa.addr(i);
      auto chr = addr == 0 ? '$' : start[addr-1];
      int idx = rank[chr];
      //printf("ch=%02x idx=%02x\n", chr, idx);
      *dest++ = idx;
      for (int i = idx; i > 0; --i) {
        mtf[i] = mtf[i-1];
        rank[mtf[i]] = i;
      }
      mtf[0] = chr;
      rank[chr] = 0;
      *dest++ = (uint8_t)idx;
    }
  }

  return dest;
}

