cmake_minimum_required(VERSION 3.20)
project(distinct_unique_words)

set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "-Werror -Wall")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-Werror -Wall")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/Wall /wd4668")
endif()

add_executable(data_generator data_generator.cpp)
add_executable(unique_words_counter unique_words_counter.cpp)
