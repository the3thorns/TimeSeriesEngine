//
// Created by thorns on 2/7/26.
//

#include "codec.h"

#include <bit>
#include <stdexcept>

#include <utils/bit.h>

namespace tsdb::core {

void GorillaCodec::compress(const TimeMark mark, BitBuffer& writer) {

  if (writer.size_bits() == 0) {
    // Prev timestamp must be the block header
    // Append header for the decompressor to do the job
    writer.append(prev_timestamp_, 64);
    prev_delta_ = mark.time - prev_timestamp_;

    writer.append(prev_delta_, 14);

    writer.append(std::bit_cast<uint64_t>(mark.value), 64);

    prev_timestamp_ = mark.time;
    prev_value_ = mark.value;
  } else {

    // Timestamp compression
    const auto new_delta = mark.time - prev_timestamp_;
    const int64_t delta_of_delta = static_cast<int64_t>(new_delta) - static_cast<int64_t>(prev_delta_);

    prev_delta_ = new_delta;
    prev_timestamp_ = mark.time;


    uint64_t value_to_write {};
    if (delta_of_delta == 0) {
      writer.append(value_to_write, 1);
    } else if (delta_of_delta >= -63 && delta_of_delta <= 64) {
      // Store '10' and the value (7 bits)
      value_to_write = 256 | delta_of_delta + 63;
      writer.append(value_to_write, 9);

    } else if (delta_of_delta >= -255 && delta_of_delta <= 256) {
      // Store '110' and the value (9 bits)
      value_to_write = 3072 | delta_of_delta + 255;
      writer.append(value_to_write, 12);

    } else if (delta_of_delta >= -2047 && delta_of_delta <= 2048) {
      // Store '1110' and the value (12 bits)
      value_to_write = 57344 | delta_of_delta + 2047;
      writer.append(value_to_write, 16);

    } else {
      // Store '1111' and the value (32 bits)
      value_to_write = 64424509440 | delta_of_delta + 4294967295;
      writer.append(value_to_write, 36);
    }

    // Value compression
    uint64_t xored_value = std::bit_cast<uint64_t>(mark.value) ^ std::bit_cast<uint64_t>(prev_value_);
    if (xored_value == 0) {
      // Prev_value == mark.value
      writer.append(0, 1);
    } else {
      const uint8_t leading_zeros = std::countl_zero(xored_value);
      const uint8_t trailing_zeros = std::countr_zero(xored_value);

      if (leading_zeros >= prev_leading_zeros_ && trailing_zeros >= prev_trailing_zeros_) {

        const uint8_t significant_bits_len = 64 - prev_trailing_zeros_ - prev_leading_zeros_;
        const uint8_t trailing_bits_to_append = significant_bits_len + 2;

        xored_value >>= prev_trailing_zeros_;
        xored_value = utils::mask_trailing_bits(xored_value, significant_bits_len);
        if (trailing_bits_to_append > 64) {
          writer.append(2, 2);
          writer.append(xored_value, significant_bits_len);
        } else {
          xored_value |= static_cast<uint64_t>(2u) << significant_bits_len;
          writer.append(xored_value, trailing_bits_to_append);
        }

      } else {
        const uint8_t significant_bits_len = 64 - trailing_zeros - leading_zeros;
        const uint8_t trailing_bits_to_append = 2 + 6 + 5 + significant_bits_len;

        xored_value >>= trailing_zeros;
        xored_value = utils::mask_trailing_bits(xored_value, significant_bits_len);

        uint64_t val = static_cast<uint64_t>(3) << 11; // 6144
        val |= leading_zeros << 6;
        val |= significant_bits_len;
        if (trailing_bits_to_append <= 64) {
          val <<= significant_bits_len;
          val |= xored_value;

          writer.append(val, trailing_bits_to_append);
        } else {
          writer.append(val, 13);
          writer.append(xored_value, significant_bits_len);
        }

        prev_leading_zeros_ = leading_zeros;
        prev_trailing_zeros_ = trailing_zeros;
      }
    }
    prev_value_ = mark.value;
  }
}

void GorillaCodec::decompress(const BitBuffer& reader, std::span<TimeMark> writer) {
  if (reader.size_bits() == 0 || writer.size_bytes() == 0) {
    return;
  }

  // Decompression context
  timestamp prev_delta {};
  timestamp prev_timestamp {};
  int64_t prev_delta_of_delta {};

  uint64_t prev_value;
  uint8_t prev_leading_zeros {64};
  uint8_t prev_trailing_zeros {64};
  // end: Decompression context

  auto iter = reader.cbegin();
  const auto end = reader.cend();
  int writer_index {};
  TimeMark tm {};
  // Retrieve header
  const timestamp header = iter.next(64);

  if (iter != end) {
    prev_delta = iter.next(14);
    prev_timestamp = prev_delta + header;
    prev_value = iter.next(64);

    tm = {prev_timestamp, std::bit_cast<double>(prev_value)};
    writer[writer_index] = tm;
    writer_index += 1;
  }

  while (iter != end) {
    timestamp decompressed_timestamp {};

    // Decompress timestamp
    int64_t delta_of_delta{};

    std::byte bit {};
    auto ones_found = 0;
    for (; ones_found < 4; ones_found += 1) {
      bit = iter.next_bit();
      if (bit == std::byte{0}) {
        break;
      }
    }

    switch (ones_found) {
      case 0: {
        delta_of_delta = prev_delta_of_delta;
        decompressed_timestamp = delta_of_delta + prev_timestamp + prev_delta;
        break;
      }
      case 1: {
        delta_of_delta = static_cast<int64_t>(iter.next(7)) - 63;
        decompressed_timestamp = delta_of_delta + prev_timestamp + prev_delta;
        prev_delta_of_delta = delta_of_delta;
        break;
      }
      case 2: {
        delta_of_delta = static_cast<int64_t>(iter.next(9)) - 255;
        decompressed_timestamp = delta_of_delta + prev_timestamp + prev_delta;
        prev_delta_of_delta = delta_of_delta;
        break;
      }
      case 3: {
        delta_of_delta = static_cast<int64_t>(iter.next(12)) - 2047;
        decompressed_timestamp = delta_of_delta + prev_timestamp + prev_delta;
        prev_delta_of_delta = delta_of_delta;
        break;
      }
      case 4: {
        delta_of_delta = static_cast<int64_t>(iter.next(32)) - 4294967295;
        decompressed_timestamp = delta_of_delta + prev_timestamp + prev_delta;
        prev_delta_of_delta = delta_of_delta;
        break;
      }
      default: throw std::runtime_error{"GorillaCodec::decompress: Compression is corrupted, wrong number of ones found in timestamp decompression"};
    }

    prev_delta = decompressed_timestamp - prev_timestamp;
    prev_timestamp = decompressed_timestamp;

    // Decompress value
    for (ones_found = 0; ones_found < 2; ones_found += 1) {
      bit = iter.next_bit();
      if (bit == std::byte{0}) {
        break;
      }
    }

    switch (ones_found) {
      case 0: { // When no ones are found, that means that prev_value does not change
        break;
      }

      case 1: { //
        auto xored = iter.next(64 - prev_trailing_zeros - prev_leading_zeros);
        xored <<= prev_trailing_zeros;

        prev_value = xored ^ prev_value;
        break;
      }

      case 2: { // Complete value stored
        const auto leading_zeros = iter.next(5);
        const auto length_of_value = iter.next(6);
        auto xored = iter.next(length_of_value);

        prev_trailing_zeros = 64 - leading_zeros - length_of_value;
        xored <<= prev_trailing_zeros;
        prev_value = xored ^ prev_value;

        prev_leading_zeros = leading_zeros;

        break;
      }
      default: throw std::runtime_error{"GorillaCodec::decompress: Compression buffer is corrupted, wrong number of ones found in value decompression"};

    }

    tm = {prev_timestamp, std::bit_cast<double>(prev_value)};
    writer[writer_index] = tm;
    writer_index += 1;
  }
}

}  // namespace tsdb::core
