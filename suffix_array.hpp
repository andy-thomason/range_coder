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


    typedef std::pair<addr_t, addr_t> sorter_t;
    std::vector<sorter_t> sorter;
    sorter.reserve(size+1);
    rank_.resize(size+1);

    {
      auto t0 = std::chrono::high_resolution_clock::now();
      addr_t acc = 0;
      sorter.emplace_back(addr_t(size), addr_t(0));
      for (auto p = begin; p != begin+4; ++p) {
        acc = acc * 0x100 + (*p & 0xff);
      }
      for (auto p = begin+4; p != end; ++p) {
        sorter.emplace_back(acc, addr_t(sorter.size()));
        acc = acc * 0x100 + (*p & 0xff);
      }
      for (int i = 0; i != 4; ++i) {
        sorter.emplace_back(acc, addr_t(sorter.size()));
        acc *= 100;
      }
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

    for (addr_t h = 1; h < size; h *=2) {
      auto t0 = std::chrono::high_resolution_clock::now();
      if (debug_full) {
        for (size_t i = 0; i != size+1; ++i) {
          addr_t string_pos = rank_[sorter[i].second];
          addr_t next_group = addr_[i] + h < size+1 ? sorter[rank_[sorter[i].second+h]].first : 0;
          char tmp[11];
          int dest = 0;
          for (auto p = begin + addr_[i]; p != end && dest != 10; ++p) {
            tmp[dest++] = *p < ' ' || *p > '~' ? '.' : *p;
          }
          tmp[dest] = 0;
          printf("%04x: g=%04x a=%04x r=%04x ng=%04x h=%04x %s\n", int(i), int(sorter[i].first), int(sorter[i].second), string_pos, next_group, h, tmp);
        }
      }

      size_t num_sorts = 0;
      size_t tot_sorts = 0;
      bool more_work_to_do = false;
      /*
      for (size_t i = 0; i != size + 1; ) {
        addr_t group = groups[i];
        size_t j = i + 1;

        while (j != size+1 && groups[j] == group) {
          ++j;
        }

        if (j != i+1) {
          num_sorts++;
          tot_sorts += j - i;
          sorter.resize(0);
          for (size_t k = i; k != j; ++k) {
            addr_t next_group = addr_[k] + h < size+1 ? groups[rank_[addr_[k]+h]] : 0;
            sorter.emplace_back(next_group, addr_[k]);
          }

          std::sort(sorter.begin(), sorter.end());

          for (size_t k = i; k != j; ) {
            addr_t next_group = sorter[k-i].first;
            groups[k] = k;
            addr_t addr = addr_[k] = sorter[k-i].second;
            rank_[addr] = k;
            size_t m = k + 1;
            while (m < j && sorter[m-i].first == next_group) {
              addr_t addr = addr_[m] = sorter[m-i].second;
              rank_[addr] = m;
              groups[m] = k;
              ++m;
              more_work_to_do = true;
            }
            k = m;
          }
        }
        i = j;
      }*/

      if (debug_full || debug_stats) {
        auto t1 = std::chrono::high_resolution_clock::now();
        printf("h=%05x  %d sorts  %d values sorted  %f ave.\n", int(h), int(num_sorts), int(tot_sorts), 1.0 * tot_sorts / num_sorts);
        printf(" t: %d\n", int(std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()));
      }
      if (!more_work_to_do) break;
    } // h
  }

  size_t size() const { return addr_.size(); }
  addr_t addr(size_t i) const { return addr_[i]; }
  addr_t rank(size_t i) const { return rank_[i]; }
  
private:
  // map pattern to string
  std::vector<addr_t, allocator_t> addr_;

  // longest common prefix
  //std::vector<addr_t, allocator_t> lcp_;

  // map string to pattern
  std::vector<addr_t, allocator_t> rank_;
};

#endif

