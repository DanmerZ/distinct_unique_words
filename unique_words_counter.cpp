#include <atomic>
#include <condition_variable>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <unordered_set>
#include <thread>
#include <future>
#include <queue>

#include <boost/unordered/concurrent_flat_set.hpp>

class ThreadSafeSet
{
public:
    void insert(const std::string& s)
    {
        std::lock_guard<std::mutex> lk(m_);
        set_.insert(s);
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lk(m_);
        return set_.size();
    }

private:
    mutable std::mutex m_;
    std::unordered_set<std::string> set_;
};

class ThreadPool
{
public:
    std::uint64_t start()
    {
        const auto number_of_threads = std::thread::hardware_concurrency();
        for (auto i = 0u; i < number_of_threads; i++)
        {
            threads_.emplace_back(std::thread{&ThreadPool::loop, this});
        } 

        return number_of_threads;
    }

    void enqueue_job(const std::function<void()>& job)
    {
        {
            std::unique_lock<std::mutex> lk(mut_);
            jobs_.push(job);
        }
        cv_.notify_one();
    }

    void stop()
    {
        {
            std::unique_lock<std::mutex> lk(mut_);
            terminate_ = true;
        }

        cv_.notify_all();

        for (auto& thread : threads_)
        {
            thread.join();
        }

        threads_.clear();
    }

private:
    void loop()
    {
        while (true)
        {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lk(mut_);
                cv_.wait(lk, [this]() {
                    return terminate_ || !jobs_.empty();
                });

                if (terminate_) {
                    return;
                }

                job = jobs_.front();
                jobs_.pop();
            }

            job();
        }
    }

    bool terminate_{ false };
    std::mutex mut_;
    std::condition_variable cv_;
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> jobs_;
};

class UniqueWordsCounter
{
public:
    explicit UniqueWordsCounter(const char* file_name, std::uintmax_t memory_limit_bytes = 1'000'000'000ull) 
        : file_name_(file_name),
        memory_limit_bytes_(memory_limit_bytes)
    {
    }

    std::uintmax_t count()
    {
        const auto file_size = std::filesystem::file_size(file_name_);

        ThreadPool tp;
        const auto number_of_threads = tp.start();

        const auto block_size = file_size > memory_limit_bytes_ ?
            memory_limit_bytes_ / number_of_threads :
            file_size / number_of_threads;

        const auto block_indices = find_block_indices(block_size, file_size);
        
        std::ifstream ifs(file_name_);

        for (auto i = 0u; i < block_indices.size() - 1; i++)
        {
            const auto buffer_size = block_indices[i + 1] - block_indices[i];

            {
                std::unique_lock<std::mutex> lk(mut_);
                cv_.wait(lk, [this]() {
                    return current_buffer_size_ < memory_limit_bytes_;
                });
            }

            std::string buffer(buffer_size, '\0');
            current_buffer_size_ += static_cast<std::intmax_t>(buffer_size);

            if (ifs.read(&buffer[0], buffer_size))
            {
                tp.enqueue_job(
                    [buf = std::move(buffer), this]() mutable {
                        const auto buf_size = buf.size();
                        split_words_and_insert(std::move(buf));
                        current_buffer_size_ -= buf_size;
                        cv_.notify_one();
                    }
                );
            }
        }

        tp.stop();
        return safe_set_.size() - 1;
    }

private:
    std::vector<std::uintmax_t> find_block_indices(const std::size_t block_size, const std::uintmax_t file_size) const
    {
        std::ifstream ifs(file_name_);

        auto seek = block_size;
        std::vector<std::uintmax_t> v;
        v.push_back(0);
        std::string word;

        ifs.seekg(0);

        while (ifs.good())
        {
            ifs.seekg(seek, std::ios::cur);
            ifs >> word;
            auto cur = ifs.tellg();
            if (cur > 0)
            {
                v.push_back(cur);
            }

            if (file_size - cur < block_size)
            {
                v.push_back(file_size - 1);
                break;
            }
        }

        return v;
    }

    void split_words_and_insert(std::string buffer)
    {
        std::stringstream ifs(buffer);
        std::string word;
        while (std::getline(ifs, word, ' '))
        {
            safe_set_.insert(word);
        }
    }

    const char* file_name_;
    const std::uintmax_t memory_limit_bytes_;
    
    boost::concurrent_flat_set<std::string> safe_set_;
    std::atomic_intmax_t current_buffer_size_ = 0;
    std::mutex mut_;
    std::condition_variable cv_;
};

std::vector<std::uintmax_t> find_block_offsets(const char* file_name, const std::size_t block_size, const std::uintmax_t file_size)
{
    std::ifstream ifs(file_name);

    auto seek = block_size;
    std::vector<std::uintmax_t> v;
    v.push_back(0);
    std::string word;

    ifs.seekg(0);

    while (ifs.good())
    {
        ifs.seekg(seek, std::ios::cur);
        ifs >> word;
        auto cur = ifs.tellg();
        if (cur > 0)
        {
            v.push_back(cur);
        }

        if (file_size - cur < block_size)
        {
            v.push_back(file_size - 1);
            break;
        }
    }

    return v;
}

void trivial_solution(const char* file_name)
{
    std::ifstream ifs(file_name);
    std::unordered_set<std::string> unique_words;

    std::string word;
    while (std::getline(ifs, word, ' '))
    {
        unique_words.insert(word);
    }

    std::cout << unique_words.size() - 1 << std::endl;
}

template<typename ThreadSafeHashSet>
void process_block(std::string buffer, ThreadSafeHashSet& safe_set)
{
    std::stringstream ifs(buffer);
    std::string word;
    while (std::getline(ifs, word, ' '))
    {
        safe_set.insert(word);        
    }
}

void block_solution(const char* file_name)
{
    const auto file_size = std::filesystem::file_size(file_name);
    const auto num_threads = std::thread::hardware_concurrency();
    const auto block_size = file_size / num_threads;

    const auto block_indices = find_block_offsets(file_name, block_size, file_size);

    ThreadSafeSet safe_set;

    std::ifstream ifs(file_name);

    for (auto i = 0; i < block_indices.size() - 1; i++)
    {
        const auto buffer_size = block_indices[i + 1] - block_indices[i];
        std::string buffer(buffer_size, '\0');
        if (ifs.read(&buffer[0], buffer_size))
        {
            process_block(std::move(buffer), safe_set);
        }        
    }

    std::cout << safe_set.size() - 1 << std::endl;
}

void block_async_solution(const char* file_name)
{
    const auto file_size = std::filesystem::file_size(file_name);
    const auto num_threads = std::thread::hardware_concurrency();
    const auto block_size = file_size / num_threads / 30;

    const auto block_indices = find_block_offsets(file_name, block_size, file_size);

    const auto buffer_size_limit = 0.1 * file_size;

    //ThreadSafeSet safe_set;
    boost::concurrent_flat_set<std::string> safe_set;    

    //std::vector<std::future<void>> futures;

    std::atomic_intmax_t current_buffer_size = 0;
    std::mutex mut;
    std::condition_variable cv;

    std::ifstream ifs(file_name);

    ThreadPool tp;
    tp.start();

    for (auto i = 0; i < block_indices.size() - 1; i++)
    {
        const auto buffer_size = block_indices[i + 1] - block_indices[i];
        
        {
            std::unique_lock<std::mutex> lk(mut);
            cv.wait(lk, [&buffer_size, &current_buffer_size, &buffer_size_limit]() {
                return current_buffer_size + buffer_size < buffer_size_limit;
                });
        }

        std::string buffer(buffer_size, '\0');
        current_buffer_size += static_cast<std::intmax_t>(buffer_size);
        
        if (ifs.read(&buffer[0], buffer_size))
        {
            //futures.push_back(
            //    std::async(std::launch::async, [buf=std::move(buffer), &safe_set, &current_buffer_size, &cv] () mutable {
            //        const auto buf_size = buf.size();
            //        process_block(std::move(buf), safe_set);
            //        current_buffer_size -= buf_size;
            //        cv.notify_one();
            //    })
            //);

            tp.enqueue_job(
                [buf = std::move(buffer), &safe_set, &current_buffer_size, &cv]() mutable {
                    const auto buf_size = buf.size();
                    process_block(std::move(buf), safe_set);
                    current_buffer_size -= buf_size;
                    cv.notify_one();
                }
            );
        }
    }

    tp.stop();

    //for (auto& future: futures)
    //{
    //    future.get();
    //}

    std::cout << safe_set.size() - 1 << std::endl;
}

int main([[maybe_unused]]int argc, char const *argv[])
{
    const char* file_name = argv[1];

    if (!std::filesystem::exists(file_name)) {
        std::cerr << "Could not find a file " << file_name << std::endl;
        return -1;
    }

    //trivial_solution(file_name);
    //block_solution(file_name);
    //block_async_solution(file_name);

    UniqueWordsCounter uwc(file_name, 1'000'000'000);
    std::cout << uwc.count() << std::endl;
    return 0;
}
