add_library(TimeSeriesEngine_Core BitBuffer.cpp
        codec.cpp
        codec.h
)

target_include_directories(TimeSeriesEngine_Core PRIVATE ../)