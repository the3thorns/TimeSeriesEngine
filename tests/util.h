//
// Created by thorns on 8/7/26.
//

#ifndef TIMESERIESENGINE_UTIL_H
#define TIMESERIESENGINE_UTIL_H

#include <print>
#include <core/BitBuffer.h>
#include <utils/bit.h>

inline void print_bit_buffer(const tsdb::core::BitBuffer& buffer) {
  for (auto byte : buffer.span()) {
    for (uint8_t i = 0; i < 8; i++) {
      auto val = static_cast<uint64_t>(byte >> (7 - i));
      val = tsdb::utils::mask_trailing_bits(val, 1u);
      std::print("{}", val);
    }
    std::print(" ");
  }
  std::println("\n");
}

#endif  // TIMESERIESENGINE_UTIL_H
