cmake_minimum_required(VERSION 3.20)
project(distinct_unique_words)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(USE_BOOST_HASH_SET "Use Boost concurrent_flat_set" OFF)
option(USE_ALPHABET_OPTIMIZATION "Enable 'alphabet optimized' ThreadSafeHashSet specialization" ON)

find_package(Boost QUIET)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "-Werror -Wall")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "-Werror -Wall")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/W4 /wd4668 /EHsc")
endif()

add_executable(data_generator src/data_generator.cpp)
add_executable(unique_words_counter 
    src/main.cpp
    src/unique_words_counter.h
    src/unique_words_counter.cpp
    src/threadpool.h
    src/threadpool.cpp
    src/thread_safe_hash_set.h
)
# target_link_libraries(unique_words_counter /opt/homebrew/lib/libprofiler.dylib)

if(USE_ALPHABET_OPTIMIZATION)
    target_compile_definitions(unique_words_counter PUBLIC USE_ALPHABET_OPTIMIZATION)
endif()

if(USE_BOOST_HASH_SET AND Boost_FOUND)
    target_include_directories(unique_words_counter PUBLIC ${Boost_INCLUDE_DIR})
    target_compile_definitions(unique_words_counter PUBLIC USE_BOOST_HASH_SET)
endif()

