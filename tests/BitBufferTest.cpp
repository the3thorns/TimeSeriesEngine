//
// Created by thorns on 24/6/26.
//

#include <core/BitBuffer.h>

#include <cstdio>

#include "gtest/gtest.h"

namespace tsdb::core {
struct TestConstIterator {
  const std::byte* pointer{};
  std::size_t bit_index {};

  explicit TestConstIterator(const BitBuffer::ConstIterator& iterator) : pointer{iterator.pointer_}, bit_index{iterator.bit_index_} {}

  TestConstIterator& operator=(const BitBuffer::ConstIterator& iterator) {
    pointer = iterator.pointer_;
    bit_index = iterator.bit_index_;
    return *this;
  }

  bool operator==(const BitBuffer::ConstIterator& iterator) const {
    return iterator.bit_index_ == bit_index && iterator.pointer_ == pointer;
  }
};
}

TEST(BitBufferTest, BaseConstructor) {
  tsdb::core::BitBuffer b{};
  ASSERT_TRUE(b.is_empty());
}

TEST(BitBufferTest, ReserveConstructor) {
  tsdb::core::BitBuffer b{4};
  ASSERT_TRUE(b.is_empty()) << "Buffer is not empty after creation";

  size_t q = b.capacity_bytes();
  ASSERT_EQ(q, 4) << "Buffer has not 4 capacity in bytes, returned " << q;

  q = b.size_bytes();
  ASSERT_EQ(q, 0) << "Buffer size should be 0 bytes, returned " << q;

  q = b.size_bits();
  ASSERT_EQ(q, 0) << "Buffer size should be 0 bits, returned " << q;
}

TEST(BitBufferTest, Append) {
  tsdb::core::BitBuffer b{64}; // Reserve 64 bytes

  size_t q = b.size_bits();
  ASSERT_EQ(q, 0) << "Expected size_bits of 0, returned " << q;

  q = b.size_bytes();
  ASSERT_EQ(q, 0) << "Expected size_bytes of 0, returned " << q;

  b.append(1, 8);

  q = b.size_bytes();
  EXPECT_EQ(q, 1) << "Expected size of 1, returned " << q;

  q = b.size_bits();
  EXPECT_EQ(q, 8) << "Expected size of 8, returned " << q;

  b.append(1, 8);

  q = b.size_bytes();
  EXPECT_EQ(q, 2) << "Expected size of 2, returned " << q;

  q = b.size_bits();
  EXPECT_EQ(q, 16) << "Expected size of 16, returned " << q;

  b.append(1, 7);

  q = b.size_bytes();
  EXPECT_EQ(q, 3) << "Expected size of 3, returned " << q;

  q = b.size_bits();
  EXPECT_EQ(q, 23) << "Expected size of 23, returned " << q;

  b.append(0xFFFF, 15);

  q = b.size_bytes();
  EXPECT_EQ(q, 5) << "Expected size of 3, returned " << q;

  q = b.size_bits();
  EXPECT_EQ(q, 38) << "Expected size of 23, returned " << q;

  auto span = b.span();
  std::byte value = span[0];

  //for (auto b : span) {
  //  std::cout << static_cast<int>(b) << " ";
  //}
  //std::cout << std::endl;

  EXPECT_EQ(value, std::byte{1})
      << "Span[0] is not equal to 1, returned " << static_cast<int>(value);

  value = span[1];
  EXPECT_EQ(value, std::byte{1})
      << "Span[1] is not equal to 1, returned " << static_cast<int>(value);

  value = span[2];
  EXPECT_EQ(value, std::byte{3})
      << "Span[1] is not equal to 2 returned " << static_cast<int>(value);

  value = span[3];
  EXPECT_EQ(value, std::byte{255})
      << "Span[1] is not equal to 15, returned " << static_cast<int>(value);

  value = span[4];
  EXPECT_EQ(value, std::byte{252})
      << "Span[1] is not equal to 14, returned " << static_cast<int>(value);
}

TEST(BitBufferTest, IteratorPeek) {
  tsdb::core::BitBuffer b{64};
  // Using the same bit arrangement as the previous TEST
  b.append(1, 8);
  b.append(1, 8);
  b.append(1, 7);
  b.append(0xFFFF, 15);

  //auto span_b = b.span();
  //for (auto byte : span_b) {
  //  std::cout << static_cast<int>(byte) << " ";
  //}
  //std::cout << std::endl;

  auto iter = b.cbegin();
  uint64_t iexpected {};

  EXPECT_THROW(iexpected = iter.peek(255), std::runtime_error) << "std::runtime_error expected";

  iexpected = iter.peek(8);
  EXPECT_EQ(iexpected, 1) << "Expected value should be 1, returned " << iexpected;


  // Test that peek does not advance the pointer not bit_index_
  iexpected = iter.peek(8);
  EXPECT_EQ(iexpected, 1) << "Expected value should be 1, returned " << iexpected;


  iexpected = iter.peek(1);
  EXPECT_EQ(iexpected, 0) << "Expected value should be 0, returned " << iexpected;

  // When bits == 0, then iexpected.value() should be 0
  // This should be defined behaviour
  iexpected = iter.peek(0);
  EXPECT_EQ(iexpected, 0) << "Expected value should be 0, returned " << iexpected;

  iexpected = iter.peek(16);
  EXPECT_EQ(iexpected, 257) << "Expected value should be 257, returned " << iexpected;

  iexpected = iter.peek(24);
  EXPECT_EQ(iexpected, 65795) << "Expected value should be 65795, returned " << iexpected;

  // This case should also be tested
  // It might be possible that some function accidentally inputs -1 in peek
  // This should return an error, because its nonsense
  EXPECT_THROW(iexpected = iter.peek(-1), std::runtime_error) << "std::runtime_error expected";
}

TEST(BitBufferTest, IteratorNext) {
  tsdb::core::BitBuffer b{64};
  // Using the same bit arrangement as the previous TEST
  b.append(1, 8);
  b.append(1, 8);
  b.append(1, 7);
  b.append(0xFFFF, 15);

  auto iter = b.cbegin();
  auto value = iter.peek(8);
  EXPECT_EQ(value, 1) << "Expected value should be 1, returned " << value;

  value = iter.next(8);
  EXPECT_EQ(value, 1) << "Expected value should be 1, returned " << value;
  tsdb::core::TestConstIterator test_iter{iter};
  EXPECT_TRUE(test_iter == iter && test_iter.bit_index == 8) << "Test iterator failed to check the attributes of iterator";

  value = iter.next(8);
  test_iter = iter;
  EXPECT_EQ(value, 1) << "Expected value should be 1, returned " << value;
  EXPECT_TRUE(test_iter == iter && test_iter.bit_index == 16) << "Test iterator failed to check the attributes of iterator";

  value = iter.next(7);
  test_iter = iter;
  EXPECT_EQ(value, 1) << "Expected value should be 1, returned " << value;
  EXPECT_TRUE(test_iter == iter && test_iter.bit_index == 23) << "Test iterator failed to check the attributes of iterator";

  value = iter.next(1);
  test_iter = iter;
  EXPECT_EQ(value, 1) << "Expected value should be 1, returned " << value;
  EXPECT_TRUE(test_iter == iter && test_iter.bit_index == 24) << "Test iterator failed to check the attributes of iterator";
}

TEST(BitBufferTest, IteratorNextBit) {
  tsdb::core::BitBuffer b {};
  // Using the same bit arrangement as the previous TEST
  b.append(1, 8);
  b.append(1, 8);
  b.append(1, 7);
  b.append(0xFFFF, 15);

  auto iter = b.cbegin();
  const auto end = b.cend();

  EXPECT_TRUE(iter != end) << "b.cbegin() and b.cend() return the same value although the buffer is not empty";

  std::byte bit{};

  for (int j = 0; j < 2; j++) {
    for (int i = 0; i < 7; i++) {
      bit = iter.next_bit();
      EXPECT_EQ(bit, std::byte{0}) << "Expected 0, returned " << static_cast<int>(bit);
    }

    bit = iter.next_bit();
    EXPECT_EQ(bit, std::byte{1}) << "Expected 1, returned " << static_cast<int>(bit);
  }

  auto value = iter.next(8);
  EXPECT_EQ(value, 3) << "Expected 3, returned " << value;
}
