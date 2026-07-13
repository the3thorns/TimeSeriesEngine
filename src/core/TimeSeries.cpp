//
// Created by thorns on 10/7/26.
//

#include "TimeSeries.h"

namespace tsdb::core {

TimeSeries::TimeSeries(std::unique_ptr<TimeSeriesConcept>&& other_pimpl) noexcept
  : pimpl_{std::move(other_pimpl)} {}

void TimeSeries::append(TimeMark mark) {
  pimpl_->append(mark);
}

std::vector<TimeMark> TimeSeries::query_last_open_block() {
  return pimpl_->query_last_open_block();
}

}  // namespace tsdb::core
