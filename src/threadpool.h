#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
  ThreadPool() = default;
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;

  ThreadPool& operator=(ThreadPool& other) = delete; 
  ThreadPool& operator=(ThreadPool&& other) = delete; 

  ~ThreadPool();

  std::uint64_t start();
  void enqueue_job(const std::function<void()> &job);
  void stop();

private:
  void loop();

  bool stop_{false};
  std::mutex mut_;
  std::condition_variable cv_;
  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> jobs_;
};
