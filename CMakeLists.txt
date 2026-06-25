cmake_minimum_required(VERSION 4.2)
project(TimeSeriesEngine)

set(CMAKE_CXX_STANDARD 23)

add_executable(TimeSeriesEngine)
target_sources(TimeSeriesEngine
    PRIVATE
        main.cpp
)

add_subdirectory(src)
target_link_libraries(TimeSeriesEngine PRIVATE TimeSeriesEngine_Core)
target_include_directories(TimeSeriesEngine PRIVATE src)

add_subdirectory("${CMAKE_SOURCE_DIR}/tests")
