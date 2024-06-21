#pragma once

#include <cstring>

namespace lvar {

  // used for converting strings to ids, wikipedia
  inline int fnv1a(char const* str)
  {
    std::size_t const len{ std::strlen(str) };
    int constexpr prime{ 0x01000193 }; // FNV-1a 32-bit prime
    unsigned int hash{ 0x811c9dc5 }; // FNV-1a 32-bit offset basis
    for(std::size_t i{ 0 }; i < len; ++i) {
      hash ^= static_cast<unsigned int>(str[i]);
      hash *= prime;
    }
    return hash;
  }

};
