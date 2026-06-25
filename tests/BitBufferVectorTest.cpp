//
// Created by thorns on 24/6/26.
//

#include <cstdio>
#include "gtest/gtest.h"
#include <core.h>


TEST(BitBufferVectorTest, BaseConstructor) {
  tsdb::core::BitBufferVector b{};
  ASSERT_TRUE(b.empty());
}

TEST(BitBufferVectorTest, ReserveConstructor) {
  tsdb::core::BitBufferVector b{4};
  ASSERT_TRUE(b.empty()) << "Buffer is not empty after creation";

  size_t q = b.capacity_bytes();
  ASSERT_EQ(q, 4) << "Buffer has not 4 capacity in bytes, returned " << q;

  q = b.size_bytes();
  ASSERT_EQ(q, 0) << "Buffer size should be 0 bytes, returned " << q;

  q = b.size_bits();
  ASSERT_EQ(q, 0) << "Buffer size should be 0 bits, returned " << q;
}

TEST(BitBufferVectorTest, Append) {
  tsdb::core::BitBufferVector b{64}; // Reserve 64 bytes
  auto err = b.append(1, 8);
  EXPECT_EQ(err, tsdb::core::BitBufferVector::Error::Ok) << "There was an error with BitBufferVector.append()";

  size_t q = b.size_bytes();
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