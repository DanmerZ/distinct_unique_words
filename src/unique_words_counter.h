#pragma once

#include <atomic>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <unordered_set>

#include "threadpool.h"
#include "thread_safe_hash_set.h"

class UniqueWordsCounter
{
public:
    explicit UniqueWordsCounter(const char* file_name, std::uintmax_t memory_limit_bytes = 1'000'000'000ull);

    std::uintmax_t count();

private:
    std::vector<std::uintmax_t> find_block_indices(const std::size_t block_size, const std::uintmax_t file_size) const;

    void split_words_and_insert(std::string buffer);

    const char* file_name_;
    const std::uintmax_t memory_limit_bytes_;
    
    ThreadSafeHashSet<std::string> safe_set_;
    std::atomic<std::uintmax_t> current_buffer_size_ = 0;
    std::mutex mut_;
    std::condition_variable cv_;
};
