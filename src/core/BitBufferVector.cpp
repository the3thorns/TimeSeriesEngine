//
// Created by thorns on 24/6/26.
//

#include "BitBufferVector.h"

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
std::uint8_t bit_count) {
  if (bit_count > sizeof(value) * 8) return Error::WrongBitSize;

  size_t new_size_bits = size_bits_ + static_cast<size_t>(bit_count);
  size_t size_bits_accum = size_bits_;
  uint8_t bit_index = size_bits_ % 8;

  while (size_bits_accum < new_size_bits) {
    size_t rel = 8 - bit_index;
    size_t diff =  new_size_bits - size_bits_accum;
    const size_t step = std::min(diff, rel);

    size_t shifted_steps{0};
    if (rel <= diff) {
      shifted_steps = diff - rel;
    } else {
      shifted_steps = rel - diff;
    }

    size_t shifted_bits = bit_count >= 8 ? value >> shifted_steps : value << shifted_steps;
    uint64_t mask{0};
    if (step == 8 || bit_count < 8) {
      mask = 0xFF;
    } else {
      for (auto i = 0; i < step; i++) {
        mask <<= 1;
        mask |= 1;
      }
    }
    shifted_bits &= mask;

    // Append to vector
    if (size_bits_accum >> 3 >= buffer_.size() ) {
      buffer_.push_back(static_cast<std::byte>(shifted_bits));
    } else {
      auto last_index = buffer_.size() - 1;
      std::byte xored_byte = buffer_[last_index];
      xored_byte ^= static_cast<std::byte>(shifted_bits);
      buffer_[last_index] = xored_byte;
    }

    size_bits_accum += step;
    bit_index = size_bits_accum % 8;
    bit_count -= step;
  }

  size_bits_ = new_size_bits;
  return Error::Ok;
}

} // core
// tsdb