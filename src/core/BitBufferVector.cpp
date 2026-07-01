//
// Created by thorns on 24/6/26.
//

#include "BitBufferVector.h"
#include <utils/bit.h>

namespace utils = tsdb::utils;

namespace tsdb::core {

BitBufferVector::BitBufferVector(const std::size_t capacity) {
  buffer_.reserve(capacity);
}

std::expected<std::byte, BitBufferVector::Error> BitBufferVector::byte_at(
    const std::size_t index) const {
  if (index >= buffer_.size()) {
    return std::unexpected{Error::OutOfBounds};
  }

  return buffer_[index];
}

std::expected<std::byte, BitBufferVector::Error> BitBufferVector::bit_at(
    const std::size_t index) const {
  if (index > size_bits_) {
    return std::unexpected{Error::OutOfBounds};
  }
  const std::size_t at = index >> 3;
  const std::size_t diff = index - (at << 3);
  std::byte targeted = buffer_[at];
  targeted >>= 8 - diff - 1;
  targeted &= std::byte{0b00000001};
  return targeted;
}

BitBufferVector::Error BitBufferVector::append(const std::uint64_t value,
                                               const std::uint8_t trailing_bits) {
  if (trailing_bits > sizeof(value) * 8) return Error::WrongBitSize;

  uint8_t bits_inserted {};
  auto bit_index { size_bits_ };

  while (bits_inserted < trailing_bits) {
    const uint8_t rel = 8 - bit_index % 8;
    const uint8_t diff = trailing_bits - bits_inserted;

    auto value_copy {value};
    size_t step {};
    if (diff >= rel) {
      bits_inserted += rel;
      step = rel;
      value_copy >>= trailing_bits - bits_inserted;
      value_copy = utils::mask_trailing_bits(value_copy, rel);

    } else {
      value_copy = utils::mask_trailing_bits(value_copy, diff);
      value_copy <<= rel - diff;
      bits_inserted += diff;
      step = diff;
    }

    // Insertion
    const auto buffer_size = buffer_.size();
    if ( bit_index >> 3 >= buffer_size  ) {
      buffer_.push_back(static_cast<std::byte>(value_copy));
    } else {
      auto last_buffer_index = buffer_size - 1;
      std::byte buffer_byte = buffer_[last_buffer_index];
      buffer_[last_buffer_index] = buffer_byte | static_cast<std::byte>(value_copy);
    }

    bit_index += step;
  }

  size_bits_ += trailing_bits;

  return Error::Ok;
}

BitBufferVector::Iterator::Iterator(std::byte* addr) : pointer_{addr} {}

BitBufferVector::Iterator::Iterator(std::byte* addr, uint8_t bit_index) : pointer_{addr}, bit_index_{bit_index} {}

std::expected<uint64_t, BitBufferVector::Error> BitBufferVector::Iterator::peek(const uint8_t bits) const {
  if (bits > sizeof(uint64_t) * 8 || bits < 0) return std::unexpected{Error::WrongBitSize};

  auto bit_index {bit_index_};
  auto pointer {pointer_};

  uint8_t bits_inserted {};

  uint64_t peeked_value {};

  while (bits_inserted < bits) {
    const uint8_t rel = 8 - bit_index % 8;
    const uint8_t diff = bits - bits_inserted;

    uint64_t buffer_byte = static_cast<uint64_t>(*pointer);
    if (diff > rel) {
      buffer_byte = utils::mask_trailing_bits(buffer_byte, rel);
      peeked_value |= buffer_byte << (diff - rel);

      bits_inserted += rel;
      pointer += 1;
      bit_index += rel;
    } else {
      buffer_byte >>= rel - diff;
      buffer_byte = utils::mask_trailing_bits(buffer_byte, diff);
      peeked_value |= buffer_byte;

      bits_inserted += diff;
    }
  }

  return peeked_value;
}

std::expected<uint64_t, BitBufferVector::Error> BitBufferVector::Iterator::next(const uint8_t bits) {
  auto expect = peek(bits);
  if (expect) {
    const auto new_bit_index = bit_index_ + bits;
    pointer_ += (new_bit_index >> 3) - (bit_index_ >> 3);
    bit_index_ += bits;
    return expect.value();
  }
  return std::unexpected{expect.error()};
}

}  // namespace tsdb::core

// tsdb