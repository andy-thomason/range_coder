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

template <class Ty1, class Ty2>
class dual_random_iterator {
public:
  dual_random_iterator(Ty1 a, Ty2 b) {
  }
private:
  Ty1 a_;
  Ty2 b_;
};


template<class value_t=std::uint8_t, class addr_t=std::uint32_t, class allocator_t=std::allocator<char>>
class suffix_array {
public:
  suffix_array(const value_t *begin, const value_t *end) {

    auto t0 = std::chrono::high_resolution_clock::now();
    constexpr bool debug_full = true;
    constexpr bool debug_stats = true;

    addr_t size = addr_t(end - begin);
    constexpr int asz = sizeof(addr_t);

    typedef std::pair<addr_t, addr_t> sorter_t;
    std::vector<sorter_t> sorter;
    sorter.reserve(size+1);
    rank_.resize(size+1);

    {
      auto t0 = std::chrono::high_resolution_clock::now();
      addr_t acc = 0;
      addr_t addr = 0;
      sorter.emplace_back(addr_t(0), addr_t(size));
      if (begin + asz < end) {
        for (auto p = begin; p != begin+asz; ++p) {
          acc = acc * 0x100 + (*p & 0xff);
        }
        for (auto p = begin+asz; p != end; ++p) {
          sorter.emplace_back(acc, addr++);
          acc = acc * 0x100 + (*p & 0xff);
        }
        for (int i = 0; i != asz; ++i) {
          sorter.emplace_back(acc, addr++);
          acc *= 0x100;
        }
      } else {
        for (auto p = begin; p != end; ++p) {
          acc = acc * 0x100 + (*p & 0xff);
        }
        for (auto p = end; p != begin + asz; ++p) {
          acc = acc * 0x100;
        }
        for (auto p = begin; p != end; ++p) {
          sorter.emplace_back(acc, addr++);
          acc = acc * 0x100;
        }
      }

      // bug note: this does not work for last asz bytes.
      std::sort(sorter.begin(), sorter.end());

      rank_[size] = 0;
      for (addr_t i = 0; i != size+1; ) {
        addr_t key = sorter[i].first;
        addr_t group = sorter[i].first = addr_t(i);
        rank_[sorter[i].second] = i;
        addr_t j = i + 1;
        while (j != size+1 && sorter[j].first == key) {
          rank_[sorter[j].second] = j;
          sorter[j].first = group;
          ++j;
        }
        i = j;
      }
      auto t1 = std::chrono::high_resolution_clock::now();
      printf("ex: %d\n", int(std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()));
    }

    for (addr_t h = asz; h < size; h *= 2) {
      if (debug_full) {
        for (size_t i = 0; i != size+1; ++i) {
          addr_t string_pos = rank_[sorter[i].second];
          addr_t next_group = sorter[i].second + h < size+1 ? sorter[rank_[sorter[i].second+h]].first : 0;
          char tmp[11];
          int dest = 0;
          for (auto p = begin + sorter[i].second; p != end && dest != 10; ++p) {
            tmp[dest++] = *p < ' ' || *p > '~' ? '.' : *p;
          }
          tmp[dest] = 0;
          printf("%08x: g=%08x a=%08x r=%08x ng=%08x h=%08x %s\n", int(i), int(sorter[i].first), int(sorter[i].second), string_pos, next_group, h, tmp);
        }
      }

      auto t0 = std::chrono::high_resolution_clock::now();
      size_t num_sorts = 0;
      size_t tot_sorts = 0;
      bool more_work_to_do = false;

      for (addr_t i = 0; i != size + 1; ) {
        addr_t group = sorter[i].first;
        addr_t j = i + 1;

        while (j != size+1 && sorter[j].first == group) {
          ++j;
        }

        if (j != i+1) {
          num_sorts++;
          tot_sorts += j - i;
          for (addr_t k = i; k != j; ++k) {
            addr_t addr = sorter[k].second;
            addr_t next_group = addr + h < size+1 ? sorter[rank_[addr + h]].first : 0;
            sorter[k].first = next_group;
            sorter[k].second = addr;
          }

          std::sort(sorter.data() + i, sorter.data() + j);

          for (addr_t k = i; k != j; ) {
            addr_t next_group = sorter[k].first;
            addr_t addr = sorter[k].second;
            sorter[k].first = k;
            rank_[addr] = k;
            addr_t m = k + 1;
            while (m < j && sorter[m].first == next_group) {
              addr_t addr = sorter[m].second;
              rank_[addr] = m;
              sorter[m].first = k;
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

    {
      size_t h = 1;
      for (size_t i = 0; i != size+1; ++i) {
        addr_t string_pos = rank_[sorter[i].second];
        addr_t next_group = sorter[i].second + h < size+1 ? sorter[rank_[sorter[i].second+h]].first : 0;
        char tmp[11];
        int dest = 0;
        for (auto p = begin + sorter[i].second; p != end && dest != 10; ++p) {
          tmp[dest++] = *p < ' ' || *p > '~' ? '.' : *p;
        }
        tmp[dest] = 0;
        printf("%08x: g=%08x a=%08x r=%08x ng=%08x h=%08x %s\n", int(i), int(sorter[i].first), int(sorter[i].second), string_pos, next_group, h, tmp);
      }
    }
  }

  size_t size() const { return addr_.size(); }
  addr_t addr(size_t i) const { return addr_[i]; }
  addr_t rank(size_t i) const { return rank_[i]; }
  
private:
  // map pattern to string
  std::vector<addr_t, allocator_t> addr_;

  // longest common prefix
  std::vector<addr_t, allocator_t> lcp_;

  // map string to pattern
  std::vector<addr_t, allocator_t> rank_;
};

#endif

