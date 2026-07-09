//
// Created by thorns on 2/7/26.
//

#ifndef TIMESERIESENGINE_TIMESERIES_H
#define TIMESERIESENGINE_TIMESERIES_H

#include <memory>

#include "types.h"
#include "codec.h"

namespace tsdb::core {

class TimeSeries {

public:
  enum class Error {
    Ok,
    AppendError,
    InvalidRange,
  };

  static constexpr int block_count = 12;
  static constexpr int block_period = 24 / block_count;

  template <TimeMarkRecurrentCodec C>
  TimeSeries() : pimpl_{ std::make_unique<TimeSeriesModel<C>>() } {}

  template <TimeMarkRecurrentCodec C>
  TimeSeries(TimeSeries&& other) noexcept : pimpl_{ std::make_unique<C>(std::move(other.pimpl_)) } {}

private:

  template <TimeMarkRecurrentCodec C>
  struct Block {
    BitBuffer buffer{};
    C codec{};
    timestamp header{};
    //size_t stored_timestamps{};

    Block() = default;
    explicit Block(timestamp header) : codec{header}, header{header} {}
  };

  struct TimeSeriesConcept {
    virtual ~TimeSeriesConcept() = default;

    virtual Error append(TimeMark mark) = 0;
    virtual Error query_range(timestamp from, timestamp to) = 0;
  };

  template <TimeMarkRecurrentCodec C>
  class TimeSeriesModel : public TimeSeriesConcept {
  public:

    TimeSeriesModel() {
      for (int i = 0; i < block_count; i+=1) {
        blocks_[i] = Block<C>{static_cast<timestamp>(i * block_period)};
      }
    }

    Error append(TimeMark mark) override {
      // Check time value
      auto hour = mark.time % 86400;
      if (hour % 2 != 0) {
        hour -= 1;
      }
      auto& block = blocks_[hour];
      auto err = block.codec.compress(mark, block.buffer);
      if (err != CodecError::Ok) {
        return Error::AppendError;
      }
      return Error::Ok;
    };

    Error query_range(const timestamp from, const timestamp to) override {
      if (from > to) return Error::InvalidRange;

      auto from_hour = from % 86400;
      auto indexed_from_hour = from_hour;
      if (indexed_from_hour % 2 == 0) {
        indexed_from_hour -= 1;
      }
      auto to_hour = to % 86400;
      auto indexed_to_hour = to_hour;
      if (indexed_to_hour % 2 == 0) {
        indexed_to_hour -= 1;
      }

      // Retrieve a range of records

      return Error::Ok;
    }

  private:
    std::array<Block<C>, block_count> blocks_;
    size_t open_blocks_{1};
    size_t first_blocK_index_{};
  };

private:
  std::unique_ptr<TimeSeriesConcept> pimpl_; // Pointer to Implementation
};

}  // namespace tsdb::core

#endif  // TIMESERIESENGINE_TIMESERIES_H
