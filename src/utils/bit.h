//
// Created by thorns on 30/6/26.
//

#ifndef TIMESERIESENGINE_BIT_H
#define TIMESERIESENGINE_BIT_H

#include <concepts>
#include <limits>
#include <cstdint>

namespace tsdb::utils {

template <typename T, typename U = uint8_t>
requires std::integral<T> && std::unsigned_integral<U>
constexpr T mask_leading_bits(T value, U bits) {
  static_assert(std::numeric_limits<U>::max() >= sizeof(T) * 8, "U cannot represent the full amount of bits of T");

  constexpr T mask {std::numeric_limits<T>::max()};
  constexpr auto bit_amount {sizeof(T) * 8};

  return value & mask << (bit_amount - bits);

}

template <typename T, typename U = uint8_t>
requires std::integral<T> && std::unsigned_integral<U>
constexpr T mask_trailing_bits(T value, U bits) {
  static_assert(std::numeric_limits<U>::max() >= sizeof(T) * 8, "U cannot represent the full amount of bits of T");

  constexpr T mask {std::numeric_limits<T>::max()};
  constexpr auto bit_amount {sizeof(T) * 8};

  return value & mask >> (bit_amount - bits);

}

template <typename T>
requires std::unsigned_integral<T>
constexpr T uceil(T n, T m) {
  return ( n + m - 1 ) / m;
}

}  // namespace: tsdb::utils
#endif  // TIMESERIESENGINE_BIT_H
