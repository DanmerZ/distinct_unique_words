#! /bin/bash

set -eu

function generate_test_data()
{
    # build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . --parallel

    # generate data: data_generator <file_size_in_bytes> <number_of_unique_words>
    ./data_generator 100000000 1000
    ./data_generator 1000000000 10000
    ./data_generator 5000000000 20000
}

function measure_time()
{
    # time mesurements
    echo "100Mb file, 1k unique words"
    time ./unique_words_counter file_1000.txt

    echo "1Gb file, 10k unique words " 
    time ./unique_words_counter file_10000.txt

    echo "5Gb file, 20k unique words"
    time ./unique_words_counter file_20000.txt
}

mkdir -p build
pushd build

generate_test_data

echo "Not-optimized ThreadSafeHashSet"
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_BOOST_HASH_SET=OFF -DUSE_ALPHABET_OPTIMIZATION=OFF ..
cmake --build . --parallel
measure_time

echo "Optimized ThreadSafeHashSet"
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_BOOST_HASH_SET=OFF -DUSE_ALPHABET_OPTIMIZATION=ON ..
cmake --build . --parallel
measure_time

echo "boost::concurrent_flat_set"
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_BOOST_HASH_SET=ON -DUSE_ALPHABET_OPTIMIZATION=OFF ..
cmake --build . --parallel
measure_time

popd