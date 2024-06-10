#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <unordered_set>
#include <thread>
#include <future>

#include <ranges>
#include <string_view>

class ThreadSafeSet
{
public:
    void insert(const std::string& s)
    {
        std::lock_guard<std::mutex> lk(m_);
        set_.insert(s);
    }

    std::size_t count() const
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

void process_block(const char* file_name, std::uintmax_t start, std::uintmax_t end, ThreadSafeSet& safe_set)
{
    std::ifstream ifs(file_name);

    ifs.seekg(start);

    std::string word;
    while (std::getline(ifs, word, ' '))
    {
        safe_set.insert(word);

        if (ifs.tellg() > end)
        {
            break;
        }
    }
}

void block_solution(const char* file_name)
{
    const auto file_size = std::filesystem::file_size(file_name);
    const auto num_threads = std::thread::hardware_concurrency();
    const auto block_size = file_size / num_threads;

    const auto block_indices = find_block_offsets(file_name, block_size, file_size);

    ThreadSafeSet safe_set;

    for (auto i = 0; i < block_indices.size() - 1; i++)
    {
        process_block(file_name, block_indices[i], block_indices[i + 1], safe_set);
    }

    std::cout << safe_set.count() - 1 << std::endl;
}

void block_async_solution(const char* file_name)
{
    const auto file_size = std::filesystem::file_size(file_name);
    const auto num_threads = std::thread::hardware_concurrency();
    const auto block_size = file_size / num_threads;

    const auto block_indices = find_block_offsets(file_name, block_size, file_size);

    ThreadSafeSet safe_set;

    std::vector<std::future<void>> futures;

    for (auto i = 0; i < block_indices.size() - 1; i++)
    {
        futures.push_back(
            std::async(std::launch::async, process_block, file_name, block_indices[i], block_indices[i + 1], std::ref(safe_set)));
    }

    for (auto& future: futures)
    {
        future.get();
    }

    std::cout << safe_set.count() - 1 << std::endl;
}

void trivial_solution_ranges(const char* file_name)
{
    std::ifstream ifs(file_name);
    std::string the_whole_text(std::istreambuf_iterator<char>{ifs}, {});

    auto view = the_whole_text | std::views::split(' ') |
        std::views::transform([](auto &&r) {
            return std::string_view(r.begin(), r.end());
        });

    std::unordered_set<std::string> unique_words;

    for (const auto word : view) {
        if (!word.empty() && word[0] != ' ') {
            unique_words.insert(std::string(word));
        }
    }

    std::cout << unique_words.size() << std::endl;
}

void buffer_io_solution(const char* file_name)
{
    std::ifstream ifs(file_name);
    ThreadSafeSet unique_words;

    const std::size_t buffer_size = 100;
    std::string buffer{buffer_size, '\0'};

    ifs.seekg(0);

    while (ifs.read(&buffer[0], buffer_size))
    {
        std::cout << buffer << std::endl;
        std::cout << buffer.size() << std::endl;
    }

}

int main([[maybe_unused]]int argc, char const *argv[])
{
    const char* file_name = argv[1];

    if (!std::filesystem::exists(file_name)) {
        std::cerr << "Could not find a file " << file_name << std::endl;
        return -1;
    }

    // trivial_solution(file_name);
    // trivial_solution_ranges(file_name);
    // buffer_io_solution(file_name);
    block_async_solution(file_name);
    return 0;
}
