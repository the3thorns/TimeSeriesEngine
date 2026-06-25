//
// Created by thorns on 24/6/26.
//

#ifndef TIMESERIESENGINE_CORE_BITBUFFERVECTOR_H
#define TIMESERIESENGINE_CORE_BITBUFFERVECTOR_H

#include <vector>
#include <span>
#include <cstdint>
#include <limits>
#include <expected>
#include <concepts>

namespace tsdb::core {

class BitBufferVector {
public:

  enum class Error : std::uint8_t {
    Ok,
    OutOfBounds,
    WrongBitSize,
  };

  BitBufferVector() = default;
  explicit BitBufferVector(std::size_t capacity);

  // bit_count represents the number of significant bits
  // 0b00111111 The number of significant bits is 6 in this case, but it could be less
  //     ^^^^^^
  [[nodiscard]] Error append(std::uint64_t value, std::uint8_t bit_count);

  [[nodiscard]] std::size_t size_bytes() const noexcept { return buffer_.size(); }
  [[nodiscard]] std::size_t size_bits() const noexcept { return size_bits_; }
  [[nodiscard]] std::uint8_t size_remainder() const noexcept { return static_cast<std::uint8_t>(size_bits_ - buffer_.size()); }
  [[nodiscard]] std::size_t capacity_bytes() const noexcept { return buffer_.capacity(); }
  [[nodiscard]] bool empty() const noexcept { return size_bits_ == 0; }

  [[nodiscard]] std::span<const std::byte> span() const noexcept { return std::span{buffer_}; }
  [[nodiscard]] std::expected<std::byte, Error> byte_at(std::size_t index) const;
  [[nodiscard]] std::expected<std::byte, Error> bit_at(std::size_t index) const;

private:
  std::size_t size_bits_{};
  std::vector<std::byte> buffer_{};
};

} // namespace tsdb::core


#endif  // TIMESERIESENGINE_CORE_BITBUFFERVECTOR_H
