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

  static constexpr int block_count = 12;
  static constexpr int block_period = 24 / block_count;

  template <RecurrentCodec C>
  TimeSeries() : pimpl_{ std::make_unique<TimeSeriesModel<C>>() } {}

  template <RecurrentCodec C>
  TimeSeries(TimeSeries&& other) noexcept : pimpl_{ std::make_unique<C>(std::move(other.pimpl_)) } {}

private:

  template <RecurrentCodec C>
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

    virtual void append(TimeMark mark) = 0;
    virtual void query_range(timestamp from, timestamp to) = 0;
  };

  template <RecurrentCodec C>
  class TimeSeriesModel : public TimeSeriesConcept {
  public:

    TimeSeriesModel() {
      for (int i = 0; i < block_count; i+=1) {
        blocks_[i] = Block<C>{static_cast<timestamp>(i * block_period)};
      }
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
