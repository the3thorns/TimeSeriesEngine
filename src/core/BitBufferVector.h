//
// Created by thorns on 24/6/26.
//

#ifndef TIMESERIESENGINE_CORE_BITBUFFERVECTOR_H
#define TIMESERIESENGINE_CORE_BITBUFFERVECTOR_H

#include <vector>
#include <span>
#include <cstdint>
#include <expected>

namespace tsdb::core {

class BitBufferVector {
public:
  enum class Error : std::uint8_t {
    Ok,
    OutOfBounds,
    WrongBitSize,
  };
 class Iterator {
  public:
   Iterator() = default;
   explicit Iterator(std::byte* addr);
   Iterator(std::byte* addr, uint8_t bit_index);

   // Peeks the next bits, but it does not advance the iterator
   [[nodiscard]] std::expected<uint64_t, Error> peek(uint8_t bits) const;

   // Peeks and advances the iterator
   [[nodiscard]] std::expected<uint64_t, Error> next(uint8_t bits);

   bool operator==(const Iterator& other) const {
     return pointer_ == other.pointer_ && bit_index_ == other.bit_index_;
   }

   bool operator!=(const Iterator& other) const {
     return pointer_ != other.pointer_ || bit_index_ != other.bit_index_;
   }

  private:
   std::byte* pointer_{};
   std::size_t bit_index_{};
/*
  testing:
*/
   friend struct TestIterator;
 };

 BitBufferVector() = default;
  explicit BitBufferVector(std::size_t capacity);

  // bit_count represents the number of significant bits
  // 0b00111111 The number of significant bits is 6 in this case, but it could be less
  //     ^^^^^^
  [[nodiscard]] Error append(std::uint64_t value, std::uint8_t bit_count);

  std::size_t size_bytes() const noexcept { return buffer_.size(); }
  std::size_t size_bits() const noexcept { return size_bits_; }
  std::uint8_t size_remainder() const noexcept { return static_cast<std::uint8_t>(size_bits_ - buffer_.size()); }
  std::size_t capacity_bytes() const noexcept { return buffer_.capacity(); }
  bool is_empty() const noexcept { return size_bits_ == 0; }

  std::span<const std::byte> span() const noexcept { return std::span{buffer_}; }
  std::expected<std::byte, Error> byte_at(std::size_t index) const;
  std::expected<std::byte, Error> bit_at(std::size_t index) const;

  Iterator begin() noexcept { return Iterator{&buffer_[0]}; }
  Iterator end() noexcept { return Iterator{&buffer_[buffer_.size() - 1], size_remainder()}; }

private:
  std::size_t size_bits_{};
  std::vector<std::byte> buffer_{};
};

} // namespace tsdb::core


#endif  // TIMESERIESENGINE_CORE_BITBUFFERVECTOR_H
