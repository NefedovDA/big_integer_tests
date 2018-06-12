cmake_minimum_required(VERSION 2.8)

project(BIGINT)

include_directories(${BIGINT_SOURCE_DIR})

add_executable(big_integer_testing
               big_integer_testing.cpp
               big_integer.h
               big_integer.cpp
               gtest/gtest-all.cc
               gtest/gtest.h
               gtest/gtest_main.cc functions.h)
