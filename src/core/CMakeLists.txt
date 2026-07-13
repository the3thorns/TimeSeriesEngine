add_library(TimeSeriesEngine_Core
        BitBuffer.cpp
        codec.cpp
        TimeSeries.cpp
)

target_include_directories(TimeSeriesEngine_Core PRIVATE ../)