#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>

class ThreadPool
{
public:
    std::uint64_t start();
    void enqueue_job(const std::function<void()>& job);
    void stop();

private:
    void loop();    

    bool terminate_{ false };
    std::mutex mut_;
    std::condition_variable cv_;
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> jobs_;
};
