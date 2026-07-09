//
// Created by thorns on 24/6/26.
//

#ifndef TIMESERIESENGINE_CORE_BITWRITER_H
#define TIMESERIESENGINE_CORE_BITWRITER_H

#include <vector>
#include <span>
#include <cstdint>
#include <expected>

namespace tsdb::core {

class BitBuffer {

public:

  enum class Error : std::uint8_t {
    Ok,
    OutOfBounds,
    WrongBitSize,
  };

public:

 class ConstIterator {
  public:
   ConstIterator() = default;
   explicit ConstIterator(const std::byte* addr);
   ConstIterator(const std::byte* addr, size_t bit_index);

   // Peeks the next bits, but it does not advance the iterator
   [[nodiscard]] uint64_t peek(uint8_t bits) const;

   [[nodiscard]] std::byte peek_bit() const;

   // Peeks and advances the iterator
   [[nodiscard]] uint64_t next(uint8_t bits);

   std::byte next_bit();

   bool operator==(const ConstIterator& other) const {
     return pointer_ == other.pointer_ && bit_index_ == other.bit_index_;
   }

   bool operator!=(const ConstIterator& other) const {
     return pointer_ != other.pointer_ || bit_index_ != other.bit_index_;
   }

  private:
   const std::byte* pointer_{};
   std::size_t bit_index_{};
/*
  testing:
*/
   friend struct TestConstIterator;
 };

public:

  BitBuffer();
  explicit BitBuffer(std::size_t capacity);

  static constexpr size_t initial_byte_capacity = 512;

  // bit_count represents the number of significant bits
  // 0b00111111 The number of significant bits is 6 in this case, but it could be less
  //     ^^^^^^
  void append(std::uint64_t value, std::uint8_t bit_count);

  [[nodiscard]] std::size_t size_bytes() const noexcept { return buffer_.size(); }
  [[nodiscard]] std::size_t size_bits() const noexcept { return size_bits_; }
  [[nodiscard]] std::uint8_t size_remainder() const noexcept { return static_cast<std::uint8_t>(size_bits_ - buffer_.size()); }
  [[nodiscard]] std::size_t capacity_bytes() const noexcept { return buffer_.capacity(); }
  [[nodiscard]] bool is_empty() const noexcept { return size_bits_ == 0; }

  [[nodiscard]] std::span<const std::byte> span() const noexcept { return std::span{buffer_}; }
  [[nodiscard]] std::byte byte_at(std::size_t index) const;
  [[nodiscard]] std::byte bit_at(std::size_t index) const;

  [[nodiscard]] ConstIterator cbegin() const noexcept {
    if (buffer_.empty()) {
      return ConstIterator{nullptr};
    }
    return ConstIterator{&buffer_.front()};
  }
  [[nodiscard]] ConstIterator cend() const noexcept {
    if (buffer_.empty()) {
      return ConstIterator{nullptr};
    }
    return ConstIterator{&buffer_.back(), size_bits_};
  }

private:
  std::size_t size_bits_{};
  std::vector<std::byte> buffer_{};
};

} // namespace tsdb::core


#endif  // TIMESERIESENGINE_CORE_BITWRITER_H
