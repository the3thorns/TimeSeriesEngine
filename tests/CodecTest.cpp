//
// Created by thorns on 7/7/26.
//

#include <exception>
#include <vector>

#include <core/core.h>
#include "util.h"
#include "gtest/gtest.h"

namespace tsdb::core {

struct GorillaCodecTester {
  // Timestamp compression storage
  timestamp prev_timestamp {};
  uint64_t prev_delta {};

  // Value compression storage
  double prev_value {};
  uint8_t prev_leading_zeros {64};
  uint8_t prev_trailing_zeros {64};

  GorillaCodecTester() = default;
  explicit GorillaCodecTester(const GorillaCodec& codec)
    : prev_timestamp{codec.prev_timestamp_}, prev_delta{codec.prev_delta_},
      prev_value{codec.prev_value_}, prev_leading_zeros{codec.prev_leading_zeros_},
      prev_trailing_zeros{codec.prev_trailing_zeros_}
  {}

  GorillaCodecTester& operator=(const GorillaCodec& codec) {
    prev_timestamp = codec.prev_timestamp_;
    prev_delta = codec.prev_delta_;

    prev_value = codec.prev_value_;
    prev_leading_zeros = codec.prev_leading_zeros_;
    prev_trailing_zeros = codec.prev_trailing_zeros_;
    return *this;
  }
};

}

class CodecTest : public testing::Test {
protected:
  CodecTest() = default;

  tsdb::core::BitBuffer buffer {};
  tsdb::core::GorillaCodec codec {};
  tsdb::core::GorillaCodecTester tester {codec};
};


using timestamp = tsdb::core::timestamp;
using TimeMark = tsdb::core::TimeMark;

constexpr timestamp header{0};
constexpr TimeMark marks[] = {
  {1, 1},
  {2, 2},
  {3, 3},
  {4, 4},
  {5, 5},
  {10, 10}
};
constexpr size_t marks_size = 6;

TEST_F(CodecTest, GorillaConstructor) {
  ASSERT_TRUE(buffer.size_bits() == 0) << "Buffer is not empty";
  ASSERT_TRUE(tester.prev_timestamp == 0) << "Prev timestamp is not 0";

  codec = tsdb::core::GorillaCodec{header};
  tester = codec;
  ASSERT_EQ(tester.prev_timestamp, header) << "Codec could not assign header as its first timestamp";
}



TEST_F(CodecTest, GorillaCompressDecompress) {

  codec = tsdb::core::GorillaCodec{header};
  std::vector<TimeMark> output;
  output.resize(marks_size, {0, 0});

  for (auto& mark : marks) {
    ASSERT_NO_THROW(codec.compress(mark, buffer)) <<  "Exception was thrown when it shouldn't";
  }

  //ASSERT_NO_THROW(tsdb::core::GorillaCodec::decompress(buffer, output)) << "Exception was thrown when it shouldn't";
  tsdb::core::GorillaCodec::decompress(buffer, output);

  std::cout << "This is the original array:\n";
  for (const auto& [time, value] : marks) {
    std::cout << "{" << time << ", " << value << "}" << std::endl;
  }
  std::cout << "This is the compressed array\n";
  //print_bit_buffer(buffer);
  std::cout << "This is the output vector\n";
  for (const auto& [time, value] : output) {
    std::cout << "{" << time << ", " << value << "}" << std::endl;
  }

  EXPECT_EQ(marks_size, output.size()) << "Output vector and mark vector do not have the same size";
  for (size_t i = 0; i < marks_size; i+=1) {
    const auto original_mark = marks[i];
    const auto decompressed_mark = output[i];
    EXPECT_EQ(original_mark.time, decompressed_mark.time) << "Decompressed marks do not match original marks";
    EXPECT_EQ(original_mark.value, decompressed_mark.value) << "Decompressed marks do not match original marks";
  }

}
