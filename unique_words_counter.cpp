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
void process_block(std::string&& buffer, ThreadSafeHashSet& safe_set)
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
    const auto block_size = file_size / num_threads / 10;

    const auto block_indices = find_block_offsets(file_name, block_size, file_size);

    const auto buffer_size_limit = 0.1 * file_size;

    //ThreadSafeSet safe_set;
    boost::concurrent_flat_set<std::string> safe_set;    

    std::vector<std::future<void>> futures;

    std::atomic_intmax_t current_buffer_size = 0;
    std::mutex mut;
    std::condition_variable cv;

    for (auto i = 0; i < block_indices.size() - 1; i++)
    {
        const auto buffer_size = block_indices[i + 1] - block_indices[i];
        std::cout << current_buffer_size << std::endl;
        
        {
            std::unique_lock<std::mutex> lk(mut);
            cv.wait(lk, [&buffer_size, &current_buffer_size, &buffer_size_limit]() {
                return current_buffer_size + buffer_size < buffer_size_limit;
                });
        }

        std::string buffer(buffer_size, '\0');
        current_buffer_size += static_cast<std::intmax_t>(buffer_size);

        std::ifstream ifs(file_name);
        ifs.seekg(block_indices[i]);

        if (ifs.read(&buffer[0], buffer_size))
        {
            futures.push_back(
                std::async(std::launch::async, [buf=std::move(buffer), &safe_set, &current_buffer_size, &cv] () mutable {
                    const auto buf_size = buf.size();
                    process_block(std::move(buf), safe_set);
                    current_buffer_size -= buf_size;
                    cv.notify_one();
                })
            );
        }
    }

    for (auto& future: futures)
    {
        future.get();
    }

    std::cout << safe_set.size() - 1 << std::endl;
}

int main([[maybe_unused]]int argc, char const *argv[])
{
    const char* file_name = argv[1];

    if (!std::filesystem::exists(file_name)) {
        std::cerr << "Could not find a file " << file_name << std::endl;
        return -1;
    }

    // trivial_solution(file_name);
    //block_solution(file_name);
    block_async_solution(file_name);
    return 0;
}
