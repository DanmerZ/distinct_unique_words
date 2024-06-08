#include <iostream>
#include <fstream>
#include <filesystem>
#include <mutex>
#include <sstream>
#include <unordered_set>

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

void trivial_solution(const char* file_name)
{
    std::ifstream ifs(file_name);
    std::unordered_set<std::string> unique_words;

    std::stringstream ss;
    ss << ifs.rdbuf();

    std::string word;
    while (std::getline(ss, word, ' '))
    {
        // std::cout << word << std::endl;
        if (!word.empty() && word[0] != ' ') {
            unique_words.insert(word);
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

    trivial_solution(file_name);
    // buffer_io_solution(file_name);
    return 0;
}
