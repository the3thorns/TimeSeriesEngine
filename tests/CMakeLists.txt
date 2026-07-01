project(tests)
add_subdirectory(lib/googletest)

add_executable(tests BitBufferVectorTest.cpp)
target_link_libraries(tests PRIVATE
        TimeSeriesEngine_Core
        gtest
        gtest_main
)
target_include_directories(tests PRIVATE "${CMAKE_SOURCE_DIR}/src")
