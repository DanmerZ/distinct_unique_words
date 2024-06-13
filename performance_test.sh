#! /bin/bash

set -eux

mkdir -p build
pushd build

# build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel

# generate data: data_generator <file_size_in_bytes> <number_of_unique_words>
# ./data_generator 10000 10
# ./data_generator 100000 50
# ./data_generator 1000000 100
# ./data_generator 100000000 1000
# ./data_generator 1000000000 10000
# ./data_generator 5000000000 20000

# performance
# time ./unique_words_counter file_10.txt
# time ./unique_words_counter file_50.txt
# time ./unique_words_counter file_100.txt
# time ./unique_words_counter file_1000.txt
# time ./unique_words_counter file_10000.txt
time ./unique_words_counter file_20000.txt

popd