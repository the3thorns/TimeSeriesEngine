//
// Created by thorns on 2/7/26.
//

#ifndef TIMESERIESENGINE_CODEC_H
#define TIMESERIESENGINE_CODEC_H

#include <span>
#include <concepts>

#include "BitBuffer.h"
#include "types.h"

namespace tsdb::core {



template <typename T>
concept RecurrentCodec =
  requires(T c, TimeMark m, BitBuffer& b, timestamp stamp, const BitBuffer& cb, std::span<TimeMark> span)
{
  T{stamp};
  c.compress(m, b);
  c.decompress(cb, span);
};

class GorillaCodec {
public:

  GorillaCodec() = default;
  explicit GorillaCodec(const timestamp header) : prev_timestamp_{header} {}

  void compress(TimeMark mark, BitBuffer& writer);
  static void decompress(const BitBuffer& reader, std::span<TimeMark> writer);

private:
  // Timestamp compression storage
  timestamp prev_timestamp_ {};
  uint64_t prev_delta_ {};

  // Value compression storage
  double prev_value_ {};
  uint8_t prev_leading_zeros_ {64};
  uint8_t prev_trailing_zeros_ {64};

/*
testing:
*/

  friend struct GorillaCodecTester;

};


}

#endif  // TIMESERIESENGINE_CODEC_H
