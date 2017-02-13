cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(range_coder_test CXX)

add_executable(rcoder main.cpp)
target_compile_features(rcoder PRIVATE cxx_range_for)

add_executable(bcoder bcoder.cpp)
target_compile_features(bcoder PRIVATE cxx_range_for)

