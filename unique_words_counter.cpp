#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <unordered_set>

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

template<typename Stream>
std::vector<int> find_block_offsets([[maybe_unused]]const Stream& stream, [[maybe_unused]]const std::size_t block_size)
{
    std::stringstream ss;
    ss << "a dog and a man a dog and a mana dog and a mana dog and a mana dog and a man";

    int seek = 5;
    std::vector<int> v;
    v.push_back(0);
    std::string word;

    ss.seekg(0);

    while (ss.good())
    {
        ss.seekg(seek, std::ios::cur);
        ss >> word;
        auto cur = ss.tellg();
        if (cur > 0) 
        {
            v.push_back(ss.tellg());
        } 
        else 
        {
            ss.seekg(-1, std::ios::end);
            v.push_back(ss.tellg());
        }

        std::cout << ss.peek();
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

int main(int argc, char const *argv[])
{
    const char* file_name = argv[1];

    if (!std::filesystem::exists(file_name)) {
        std::cerr << "Could not find a file " << file_name << std::endl;
        return -1;
    }
    find_block_offsets(1, 10);
    trivial_solution(file_name);
    // trivial_solution_ranges(file_name);
    // buffer_io_solution(file_name);
    return 0;
}
