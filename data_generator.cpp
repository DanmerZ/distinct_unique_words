#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <unordered_set>

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
template <typename T = std::mt19937>
auto random_generator() -> T {
    // auto constexpr seed_bytes = sizeof(typename T::result_type) * T::state_size;
    // auto constexpr seed_len = seed_bytes / sizeof(std::seed_seq::result_type);
    // auto seed = std::array<std::seed_seq::result_type, seed_len>();

    // auto dev = std::random_device();
    // constexpr auto seed_len = 1;
    // auto seed = std::array<std::seed_seq::result_type, seed_len>();
    // std::generate_n(begin(seed), seed_len, std::ref(dev));
    auto seed_seq = std::seed_seq{42};
    return T{seed_seq};
}

auto generate_random_string(std::size_t len) -> std::string {
    static constexpr auto chars = "abcdefghijklmnopqrstuvwxyz";
    thread_local auto rng = random_generator<>();
    auto dist = std::uniform_int_distribution{{}, std::strlen(chars) - 1};

    auto len_dist = std::uniform_int_distribution<>{1, static_cast<int>(len)};
    auto str_length = len_dist(rng);
    auto result = std::string(str_length, '\0');

    std::generate_n(std::begin(result), len, [&]() { return chars[dist(rng)]; });
    return result;
}


int main(int argc, char const *argv[])
{
    const std::size_t max_string_length = 10;
    const std::size_t file_size_in_bytes = 1000;

    std::unordered_set<std::string> unique_words;

    std::int64_t total_size = 0;

    std::ofstream file("tmp.txt");

    while (total_size < file_size_in_bytes)
    {
        auto str = generate_random_string(max_string_length);
        unique_words.insert(str);

        str.push_back(' ');
        std::int64_t diff = file_size_in_bytes - total_size - str.size();
        if (diff < 0) {
            str = std::string(file_size_in_bytes - total_size, ' ');
        }
        total_size += str.size();
        file << str;
    }    

    std::cout << "Unique words count: " << unique_words.size() << std::endl;

    std::stringstream ss;
    ss << "file_" << unique_words.size() << ".txt";

    std::filesystem::rename("tmp.txt", ss.str());
    
    return 0;
}
