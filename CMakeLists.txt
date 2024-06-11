cmake_minimum_required(VERSION 3.20)
project(distinct_unique_words)

set(CMAKE_CXX_STANDARD 20)

set(Boost_DEBUG ON)
find_package(Boost REQUIRED)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "-Werror -Wall")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-Werror -Wall")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/W4 /wd4668 /EHsc")
endif()

add_executable(data_generator data_generator.cpp)
add_executable(unique_words_counter unique_words_counter.cpp)
# target_link_libraries(unique_words_counter /opt/homebrew/lib/libprofiler.dylib)

if(Boost_FOUND)
    target_include_directories(unique_words_counter PUBLIC ${Boost_INCLUDE_DIR})
    # target_link_libraries(unique_words_counter boost::container)
endif()
