#include <cstdint>
#include <stdio.h>
#include <array>
#include <algorithm>

// see https://en.wikipedia.org/wiki/Range_encoding

// limit the total of an array to 64k
template <class Sizes>
void limit_total_to_64k(Sizes &sizes, size_t total) {
  size_t num_symbols = sizes.size();

  int shift = 0;
  while ((total >> shift) >= 0x20000) {
    shift++;
  }

  while ((total << -shift) < 0x10000) {
    shift--;
  }

  //for (size_t i = 0; i != num_symbols; ++i) {
  //  if (sizes[i]) printf("[%02x %d]", int(i), int(sizes[i]));
  //}
  //printf("initial: total=%08lx\n", long(total));

  total = 0;
  for (size_t i = 0; i != num_symbols; ++i) {
    auto size = sizes[i];
    if (size) {
      if (shift < 0) {
        sizes[i] = size << -shift;
      } else {
        sizes[i] = (size >> shift) ? size >> shift : 1;
      }
      total += sizes[i];
    }
  }
  //printf("first pass: total=%08lx\n", long(total));

  size_t pass = 0;
  while (total > 0x10000 && pass++ < 20) {
    for (size_t i = 0; i != num_symbols && total > 0x10000; ++i) {
      auto size = sizes[i];
      if (size > 1) {
        size_t delta = size - size * 0x10000 / total;
        if (total - delta < 0x10000) {
          delta = total - 0x10000;
        }
        total -= delta; 
        sizes[i] -= delta;
      }
    }
    //for (size_t i = 0; i != num_symbols; ++i) {
    //  if (sizes[i]) printf("[%02x %d]", int(i), int(sizes[i]));
    //}
    //printf("pass %d: total=%08lx\n", int(pass), long(total));
  }
}



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

  limit_total_to_64k(sizes, size);
  constexpr size_t total = 0x10000;

  printf("calculated size=%ld / total=%ld\n", long(size), long(total));

  ctxt.size = size;
  for (uint32_t i = 0, start = 0; i != mask+1; ++i) {
    ctxt.starts[i] = start;
    start += (uint32_t)sizes[i];
  }
  ctxt.starts[mask+1] = uint32_t(total);

  constexpr int shift = 64 - 8;
  typedef uint64_t acc_t;
  acc_t low = 0;
  acc_t range = ~(uint64_t)0;
  for (auto p = begin; p != end; ++p) {
    auto start = ctxt.starts[*p & mask];
    auto size = (uint32_t)sizes[*p & mask];

    range /= total;
    low += start * range;
    range *= size;

    //printf("%02x start=%04x size=%04x\n", *p & mask, start, size);
    //printf("%02x [%04lx..%04lx] range=%016lx..%016lx [%016lx]\n", *p & mask, long(start), long(start+size), long(low), long(low+range), long(range));

    // if the top byte is the same output the byte and increase the range
    while ((low >> shift) == ((low + range) >> shift)) {
      //printf("%02x\n", uint8_t(low >> shift));
      *dest++ = uint8_t(low >> shift);
      if (dest >= destmax) return dest;
      low <<= 8;
      range <<= 8;
    }

    // if the range is too small, output some bytes and increase the range.
    if (range < 0x10000ll) {
      //printf("overflow\n");
      *dest++ = uint8_t(low >> shift);
      if (dest >= destmax) return dest;
      low <<= 8;
      *dest++ = uint8_t(low >> shift);
      if (dest >= destmax) return dest;
      low <<= 8;
      range = ~low;
    }
  }

  while (range < 0x10000) {
    *dest++ = uint8_t(low >> shift);
    if (dest >= destmax) return dest;
    low <<= 8;
    range <<= 8;
  }

  low += ((acc_t)1 << shift);
  *dest++ = uint8_t(low >> shift);

  return dest;
}

