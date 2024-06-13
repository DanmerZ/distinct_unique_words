#include "threadpool.h"

ThreadPool::~ThreadPool() { stop(); }

std::uint64_t ThreadPool::start() {
  const auto number_of_threads = std::thread::hardware_concurrency();
  for (auto i = 0u; i < number_of_threads; i++) {
    threads_.emplace_back(std::thread{&ThreadPool::loop, this});
  }

  return number_of_threads;
}

void ThreadPool::enqueue_job(const std::function<void()> &job) {
  {
    std::unique_lock<std::mutex> lk(mut_);
    jobs_.push(job);
  }
  cv_.notify_one();
}

void ThreadPool::stop() {
  {
    std::unique_lock<std::mutex> lk(mut_);
    stop_ = true;
  }

  cv_.notify_all();

  for (auto &thread : threads_) {
    thread.join();
  }

  threads_.clear();
}

void ThreadPool::loop() {
  while (true) {
    std::function<void()> job;
    {
      std::unique_lock<std::mutex> lk(mut_);
      cv_.wait(lk, [this]() { return stop_ || !jobs_.empty(); });

      if (stop_ && jobs_.empty()) {
        return;
      }

      job = jobs_.front();
      jobs_.pop();
    }

    job();
  }
}