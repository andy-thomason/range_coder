////////////////////////////////////////////////////////////////////////////////
//
// Suffix array
//
// A sorted array of substrings finishing at the end of the string.
//
// eg. "dabec" -> ["abec", "bec", "c", "dabec", "ec"]
//
// (C) Andy Thomason 2017
//
// MIT License
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _SUFFIX_ARRAY_HPP_INCLUDED
#define _SUFFIX_ARRAY_HPP_INCLUDED

#include <array>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <chrono>

template<class value_t=std::uint8_t, class addr_t=std::uint32_t, class allocator_t=std::allocator<char>>
class suffix_array {
  typedef uint64_t sorter_t;
  static addr_t adr(sorter_t v) { return (addr_t)v; }
  static addr_t grp(sorter_t v) { return (addr_t)(v >> 32); }
  static sorter_t make(addr_t g, addr_t a) { return ((sorter_t)g << 32) | a; }

public:
  suffix_array(const value_t *begin, const value_t *end) {
    auto t0 = std::chrono::high_resolution_clock::now();
    constexpr bool debug_full = false;
    constexpr bool debug_stats = false;

    addr_t size = addr_t(end - begin);
    constexpr int asz = sizeof(addr_t);

    sorter_.reserve(size+1);
    sorter_.resize(0);
    rank_.resize(size+1);

    {
      auto t0 = std::chrono::high_resolution_clock::now();
      addr_t acc = 0;
      // note: doing addr backwards makes the last asz values sort before others.
      addr_t addr = size+1;
      sorter_.emplace_back(make(addr_t(0), 0));
      if (begin + asz < end) {
        // usual case
        for (auto p = begin; p != begin+asz; ++p) {
          acc = acc * 0x100 + (*p & 0xff);
        }
        for (auto p = begin+asz; p != end; ++p) {
          sorter_.emplace_back(make(acc, --addr));
          acc = acc * 0x100 + (*p & 0xff);
        }
        for (int i = 0; i != asz; ++i) {
          sorter_.emplace_back(make(acc, --addr));
          acc *= 0x100;
        }
      } else {
        // short case
        for (auto p = begin; p != end; ++p) {
          acc = acc * 0x100 + (*p & 0xff);
        }
        // pad with zeros
        for (auto p = end; p != begin + asz; ++p) {
          acc = acc * 0x100;
        }
        for (auto p = begin; p != end; ++p) {
          sorter_.emplace_back(make(acc, --addr));
          acc = acc * 0x100;
        }
      }

      std::sort(sorter_.data(), sorter_.data() + sorter_.size());

      rank_[size] = 0;
      for (addr_t i = 0; i != size+1; ) {
        addr_t key = grp(sorter_[i]);
        addr_t group = addr_t(i);
        addr_t addr = size - adr(sorter_[i]);
        rank_[addr] = i;
        sorter_[i] = make(group, addr);
        //printf("%08x %08x %016llx\n", int(key), int(addr), (long long)(sorter_[i]));
        addr_t j = i + 1;
        // todo: handle last asz values. For example if block is all zeros
        // the last asz values should be distinct even if the key is the same.
        if (addr + asz < size) {
          while (j != size+1 && grp(sorter_[j]) == key) {
            addr = size - adr(sorter_[j]);
            rank_[addr] = j;
            sorter_[j] = make(group, addr);
            //printf("%08x %08x %016llx\n", int(key), int(addr), (long long)(sorter_[j]));
            ++j;
          }
        }
        i = j;
      }
      auto t1 = std::chrono::high_resolution_clock::now();
      if (debug_stats) printf("ex: %d\n", int(std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()));
    }

    for (addr_t h = asz; h < size; h *= 2) {
      if (debug_full) {
        for (size_t i = 0; i != size+1; ++i) {
          addr_t addr = adr(sorter_[i]);
          addr_t string_pos = rank_[addr];
          addr_t next_group = addr + h < size+1 ? grp(sorter_[rank_[addr+h]]) : 0;
          char tmp[11];
          int dest = 0;
          for (auto p = begin + addr; p != end && dest != 10; ++p) {
            tmp[dest++] = *p < ' ' || *p > '~' ? '.' : *p;
          }
          tmp[dest] = 0;
          printf("%08x: g=%08x a=%08x r=%08x ng=%08x h=%08x %s\n", int(i), int(grp(sorter_[i])), int(adr(sorter_[i])), string_pos, next_group, h, tmp);
        }
      }

      auto t0 = std::chrono::high_resolution_clock::now();
      size_t num_sorts = 0;
      size_t tot_sorts = 0;
      bool more_work_to_do = false;

      for (addr_t i = 0; i != size + 1; ) {
        addr_t group = grp(sorter_[i]);
        addr_t j = i + 1;

        while (j != size+1 && grp(sorter_[j]) == group) {
          ++j;
        }

        if (j != i+1) {
          num_sorts++;
          tot_sorts += j - i;
          for (addr_t k = i; k != j; ++k) {
            addr_t addr = adr(sorter_[k]);
            addr_t next_group = addr + h < size+1 ? grp(sorter_[rank_[addr+h]]) : 0;
            sorter_[k] = make(next_group, addr);
          }

          if (false && j - i == 20) {
            for (addr_t q = i; q != j; ++q) {
              printf("%d %016llx\n", int(q-i), (long long)(sorter_[q]));
            }
          }
          std::sort(sorter_.data() + i, sorter_.data() + j);

          for (addr_t k = i; k != j; ) {
            addr_t next_group = grp(sorter_[k]);
            addr_t addr = adr(sorter_[k]);
            sorter_[k] = make(k, addr);
            rank_[addr] = k;
            addr_t m = k + 1;
            while (m < j && grp(sorter_[m]) == next_group) {
              addr_t addr = adr(sorter_[m]);
              rank_[addr] = m;
              sorter_[m] = make(k, addr);
              ++m;
              more_work_to_do = true;
            }
            k = m;
          }
        }
        i = j;
      }

      if (debug_full || debug_stats) {
        auto t1 = std::chrono::high_resolution_clock::now();
        printf("h=%05x  %d sorts  %d values sorted  %f ave.\n", int(h), int(num_sorts), int(tot_sorts), 1.0 * tot_sorts / num_sorts);
        printf(" t: %d\n", int(std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()));
      }
      if (!more_work_to_do) break;
    } // h

    if (debug_full) {
      size_t h = 1;
      for (size_t i = 0; i != size+1; ++i) {
        addr_t string_pos = rank_[adr(sorter_[i])];
        char tmp[11];
        int dest = 0;
        for (auto p = begin + adr(sorter_[i]); p != end && dest != 10; ++p) {
          tmp[dest++] = *p < ' ' || *p > '~' ? '.' : *p;
        }
        tmp[dest] = 0;
        printf("%08x: g=%08x a=%08x r=%08x %s\n", int(i), int(grp(sorter_[i])), int(adr(sorter_[i])), int(string_pos), tmp);
      }
    }
  }

  size_t size() const { return sorter_.size(); }
  addr_t addr(size_t i) const { return adr(sorter_[i]); }
  addr_t rank(size_t i) const { return rank_[i]; }
  
private:
  // map pattern to string
  std::vector<sorter_t, allocator_t> sorter_;

  // longest common prefix
  std::vector<addr_t, allocator_t> lcp_;

  // map string to pattern
  std::vector<addr_t, allocator_t> rank_;
};

#endif

