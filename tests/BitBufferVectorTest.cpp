//
// Created by thorns on 24/6/26.
//

#include <cstdio>
#include "gtest/gtest.h"
#include <core/BitBufferVector.h>

namespace tsdb::core {
struct TestIterator {
  std::byte* pointer {};
  std::size_t bit_index {};

  explicit TestIterator(const BitBufferVector::Iterator& iterator) : pointer{iterator.pointer_}, bit_index{iterator.bit_index_} {}

  TestIterator& operator=(const BitBufferVector::Iterator& iterator) {
    pointer = iterator.pointer_;
    bit_index = iterator.bit_index_;
    return *this;
  }

  bool operator==(const BitBufferVector::Iterator& iterator) const {
    return iterator.bit_index_ == bit_index && iterator.pointer_ == pointer;
  }
};
}

TEST(BitBufferVectorTest, BaseConstructor) {
  tsdb::core::BitBufferVector b{};
  ASSERT_TRUE(b.is_empty());
}

TEST(BitBufferVectorTest, ReserveConstructor) {
  tsdb::core::BitBufferVector b{4};
  ASSERT_TRUE(b.is_empty()) << "Buffer is not empty after creation";

  size_t q = b.capacity_bytes();
  ASSERT_EQ(q, 4) << "Buffer has not 4 capacity in bytes, returned " << q;

  q = b.size_bytes();
  ASSERT_EQ(q, 0) << "Buffer size should be 0 bytes, returned " << q;

  q = b.size_bits();
  ASSERT_EQ(q, 0) << "Buffer size should be 0 bits, returned " << q;
}

TEST(BitBufferVectorTest, Append) {
  tsdb::core::BitBufferVector b{64}; // Reserve 64 bytes

  size_t q = b.size_bits();
  ASSERT_EQ(q, 0) << "Expected size_bits of 0, returned " << q;

  q = b.size_bytes();
  ASSERT_EQ(q, 0) << "Expected size_bytes of 0, returned " << q;

  auto err = b.append(1, 8);
  EXPECT_EQ(err, tsdb::core::BitBufferVector::Error::Ok) << "There was an error with BitBufferVector.append()";

  q = b.size_bytes();
  EXPECT_EQ(q, 1) << "Expected size of 1, returned " << q;

  q = b.size_bits();
  EXPECT_EQ(q, 8) << "Expected size of 8, returned " << q;

  err = b.append(1, 8);
  EXPECT_EQ(err, tsdb::core::BitBufferVector::Error::Ok) << "There was an error with BitBufferVector.append()";

  q = b.size_bytes();
  EXPECT_EQ(q, 2) << "Expected size of 2, returned " << q;

  q = b.size_bits();
  EXPECT_EQ(q, 16) << "Expected size of 16, returned " << q;

  err = b.append(1, 7);
  EXPECT_EQ(err, tsdb::core::BitBufferVector::Error::Ok) << "There was an error with BitBufferVector.append()";

  q = b.size_bytes();
  EXPECT_EQ(q, 3) << "Expected size of 3, returned " << q;

  q = b.size_bits();
  EXPECT_EQ(q, 23) << "Expected size of 23, returned " << q;

  err = b.append(0xFFFF, 15);
  EXPECT_EQ(err, tsdb::core::BitBufferVector::Error::Ok) << "There was an error with BitBufferVector.append()";

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

TEST(BitBufferVectorTest, IteratorPeek) {
  tsdb::core::BitBufferVector b{64};
  // Using the same bit arrangement as the previous TEST
  auto err = b.append(1, 8);
  err = b.append(1, 8);
  err = b.append(1, 7);
  err = b.append(0xFFFF, 15);

  //auto span_b = b.span();
  //for (auto byte : span_b) {
  //  std::cout << static_cast<int>(byte) << " ";
  //}
  //std::cout << std::endl;

  auto iter = b.begin();
  auto iexpected = iter.peek(255);
  ASSERT_FALSE(iexpected.has_value()) << "Expected object should contain an error, object does not contain an error";

  iexpected = iter.peek(8);
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 1) << "Expected value should be 1, returned " << iexpected.value();


  // Test that peek does not advance the pointer not bit_index_
  iexpected = iter.peek(8);
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 1) << "Expected value should be 1, returned " << iexpected.value();


  iexpected = iter.peek(1);
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 0) << "Expected value should be 0, returned " << iexpected.value();

  // When bits == 0, then iexpected.value() should be 0
  // This should be defined behaviour
  iexpected = iter.peek(0);
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 0) << "Expected value should be 0, returned " << iexpected.value();

  iexpected = iter.peek(16);
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 257) << "Expected value should be 257, returned " << iexpected.value();

  iexpected = iter.peek(24);
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 65795) << "Expected value should be 65795, returned " << iexpected.value();

  // This case should also be tested
  // It might be possible that some function accidentally inputs -1 in peek
  // This should return an error, because its nonsense
  iexpected = iter.peek(-1);
  ASSERT_FALSE(iexpected.has_value()) << "Expected object should contain an error, object does not contain an error";
}

TEST(BitBufferVectorTest, IteratorNext) {
  tsdb::core::BitBufferVector b{64};
  // Using the same bit arrangement as the previous TEST
  auto err = b.append(1, 8);
  err = b.append(1, 8);
  err = b.append(1, 7);
  err = b.append(0xFFFF, 15);

  auto iter = b.begin();
  auto iexpected = iter.peek(8);
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 1) << "Expected value should be 1, returned " << iexpected.value();

  iexpected = iter.next(8);
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 1) << "Expected value should be 1, returned " << iexpected.value();
  tsdb::core::TestIterator test_iter{iter};
  EXPECT_TRUE(test_iter == iter && test_iter.bit_index == 8) << "Test iterator failed to check the attributes of iterator";

  iexpected = iter.next(8);
  test_iter = iter;
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 1) << "Expected value should be 1, returned " << iexpected.value();
  EXPECT_TRUE(test_iter == iter && test_iter.bit_index == 16) << "Test iterator failed to check the attributes of iterator";

  iexpected = iter.next(7);
  test_iter = iter;
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 1) << "Expected value should be 1, returned " << iexpected.value();
  EXPECT_TRUE(test_iter == iter && test_iter.bit_index == 23) << "Test iterator failed to check the attributes of iterator";

  iexpected = iter.next(1);
  test_iter = iter;
  ASSERT_TRUE(iexpected.has_value()) << "Expected object should contain 1, object contains an error";
  EXPECT_EQ(iexpected.value(), 1) << "Expected value should be 1, returned " << iexpected.value();
  EXPECT_TRUE(test_iter == iter && test_iter.bit_index == 24) << "Test iterator failed to check the attributes of iterator";
}
