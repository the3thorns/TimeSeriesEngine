//
// Created by thorns on 2/7/26.
//

#ifndef TIMESERIESENGINE_TIMESERIES_H
#define TIMESERIESENGINE_TIMESERIES_H

#include <memory>
#include <vector>

#include "types.h"
#include "codec.h"

namespace tsdb::core {

class TimeSeries {

public:

  static constexpr int block_count = 12;
  static constexpr int block_period = 24 / block_count;

private:

  template <RecurrentCodec C>
  struct Block {
    BitBuffer buffer{};
    C codec{};
    timestamp header{};
    size_t stored_timestamps{};

    Block() = default;
    explicit Block(timestamp header) : codec{header}, header{header} {}

    void clear() {
      stored_timestamps = 0;
      buffer.clear();
      codec = C{header}; // Destroys the previous context
    }
  };

  struct TimeSeriesConcept {
    virtual ~TimeSeriesConcept() = default;

    virtual void append(TimeMark mark) = 0;
    [[nodiscard]] virtual std::vector<TimeMark> query_last_open_block() const = 0;
    //[[nodiscard]] virtual std::vector<TimeMark> query_range(timestamp from, timestamp to) const = 0;
  };

  template <RecurrentCodec C>
  class TimeSeriesModel : public TimeSeriesConcept {
  public:

    TimeSeriesModel() {
      for (int i = 0; i < block_count; i+=1) {
        blocks_[i] = Block<C>{static_cast<timestamp>(i * block_period)};
      }
    }

    void append(const TimeMark mark) override {
      auto time {mark.time};
      if (time > blocks_[open_block_index_].header) {

      }
      auto& block = blocks_[open_block_index_];
      // Append mark
      block.codec.compress(mark, block.buffer);
    }

    [[nodiscard]] std::vector<TimeMark> query_last_open_block() const override {
      auto& block = blocks_[open_block_index_];
      std::vector<TimeMark> storage{};
      storage.resize(block.stored_timestamps);

      block.codec.decompress(block.buffer, storage);
      return storage;
    }

  private:
    std::array<Block<C>, block_count> blocks_;
    size_t first_block_index_ {};
    size_t open_block_index_ {};
  };

private:
  std::unique_ptr<TimeSeriesConcept> pimpl_ {}; // Pointer to Implementation

public:

  // Creates a time series that owns nothing
  TimeSeries() = default;

  // Move constructor
  // Something that I should remember, a rvalue reference is a lvalue
  // That is why std::move should be used
  explicit TimeSeries(std::unique_ptr<TimeSeriesConcept>&& other_pimpl) noexcept;

  // Factory method used to create TimeSeries with different RecurrentCodec(s)
  template <RecurrentCodec C>
  static TimeSeries create() {
    return TimeSeries{std::make_unique<TimeSeriesModel<C>>()};
  }

  // All of this because C++ does not allow to call templated constructors without arguments
  // Templated constructor types must always be deduced from the argument list. I shall not forget that
  /*
   For example, this constructor cannot be called because template argument deduction cannot be resolved by the compiler
   There must exist some argument of type T in the constructor argument list

   template <typename T>
   TimeSeries() : pimpl_{std::make_unique<TimeSeriesModel<C>>() {} // This constructor cannot be called because there is not any T argument in the function signature
  */
  // That is why a factory method is used, because the compiler can resolve its function name (TimeSeries TimeSeries::create())
  // C++ has some weird stuff really...


  void append(TimeMark mark);
  [[nodiscard]] std::vector<TimeMark> query_last_open_block();

};

}  // namespace tsdb::core

#endif  // TIMESERIESENGINE_TIMESERIES_H
