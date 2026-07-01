//
// Created by thorns on 25/6/26.
//

#ifndef TIMESERIESENGINE_TYPES_H
#define TIMESERIESENGINE_TYPES_H

#include <cstdint>

namespace tsdb::core {

  using timestamp = uint64_t;

  struct TimeMark {
    timestamp time;
    double value;
  };

}

#endif  // TIMESERIESENGINE_TYPES_H
