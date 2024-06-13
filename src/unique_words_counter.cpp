#include "unique_words_counter.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <utility>

#include "threadpool.h"

UniqueWordsCounter::UniqueWordsCounter(const char *file_name,
                                       std::uintmax_t memory_limit_bytes)
    : file_name_(file_name), memory_limit_bytes_(memory_limit_bytes) {}

std::uintmax_t UniqueWordsCounter::count() {
  const auto file_size = std::filesystem::file_size(file_name_);

  ThreadPool thread_pool;
  const auto number_of_threads = thread_pool.start();

  const auto block_size = file_size > memory_limit_bytes_
                              ? memory_limit_bytes_ / number_of_threads
                              : file_size / number_of_threads;

  const auto block_indices = find_block_indices(block_size, file_size);

  std::ifstream ifs(file_name_);

  for (auto i = 0u; i < block_indices.size() - 1u; i++) {
    const auto buffer_size = block_indices[i + 1u] - block_indices[i];

    {
      std::unique_lock<std::mutex> lk(mut_);
      cv_.wait(lk,
               [this]() { return current_buffer_size_ < memory_limit_bytes_; });
    }

    std::string buffer(buffer_size, '\0');
    current_buffer_size_ += static_cast<std::intmax_t>(buffer_size);

    if (ifs.read(&buffer[0], buffer_size)) {
      thread_pool.enqueue_job([buf = std::move(buffer), this]() mutable {
        const auto buf_size = buf.size();
        split_words_and_insert(buf);
        buf.clear();

        current_buffer_size_ -= buf_size;
        cv_.notify_one();
      });
    }
  }

  thread_pool.stop(); // wait for threads to finish jobs

  return safe_set_.size() - 1;
}

std::vector<std::uintmax_t>
UniqueWordsCounter::find_block_indices(std::size_t block_size,
                                       std::uintmax_t file_size) const {
  std::ifstream ifs(file_name_);

  std::vector<std::uintmax_t> indices;
  indices.push_back(0);
  std::string word;

  ifs.seekg(0);

  while (ifs.good()) {
    ifs.seekg(block_size, std::ios::cur);
    ifs >> word;
    auto cur = ifs.tellg();
    if (cur > 0) {
      indices.push_back(cur);
    }

    if (file_size - cur < block_size) {
      indices.push_back(file_size - 1);
      break;
    }
  }

  return indices;
}

void UniqueWordsCounter::split_words_and_insert(const std::string &buffer) {
  std::stringstream ifs(buffer);
  std::string word;
  while (std::getline(ifs, word, ' ')) {
    safe_set_.insert(word);
  }
}
