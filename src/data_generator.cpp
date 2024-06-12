#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <unordered_set>
#include <vector>

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
template <typename T = std::mt19937>
auto random_generator() -> T {
    auto seed_seq = std::seed_seq{42};
    return T{seed_seq};
}

auto generate_random_string(int max_string_length) -> std::string {
    static constexpr auto chars = "abcdefghijklmnopqrstuvwxyz";
    thread_local auto rng = random_generator<>();
    auto dist = std::uniform_int_distribution{{}, std::strlen(chars) - 1};

    auto len_dist = std::uniform_int_distribution<>{1, max_string_length};
    auto str_length = len_dist(rng);
    auto result = std::string(str_length, '\0');

    std::generate_n(std::begin(result), str_length, [&]() { return chars[dist(rng)]; });
    return result;
}

int generate_random_int(int low, int high)
{
    thread_local auto rng = random_generator<>();
    auto dist = std::uniform_int_distribution{low, high};

    return dist(rng);
}

int main([[maybe_unused]]int argc, [[maybe_unused]]char const *argv[])
{
    const std::size_t max_string_length = 100;
    const std::size_t file_size_in_bytes = 1'000'000'000;
    const std::size_t max_unique_words_count = 20'000;

    std::unordered_set<std::string> generated_unique_words;

    while(generated_unique_words.size() < max_unique_words_count)
    {
        const auto str = generate_random_string(max_string_length);
        generated_unique_words.insert(str);
    }

    std::vector<std::string> generated_unique_words_vec{generated_unique_words.begin(), generated_unique_words.end()};

    std::unordered_set<std::string> unique_words;

    std::int64_t total_size = 0;

    {
        std::ofstream file("tmp.txt");

        while (total_size < file_size_in_bytes)
        {
            auto random_index = generate_random_int(0, static_cast<int>(generated_unique_words_vec.size()) - 1);
            auto str = generated_unique_words_vec[random_index];
            str.push_back(' ');

            // check for last word length to be within file size limit
            std::int64_t diff = file_size_in_bytes - total_size - str.size();
            if (diff < 0) {
                // replace last word by spaces to the end of file
                str = std::string(file_size_in_bytes - total_size, ' ');
            } else {
                unique_words.insert(str);
            }

            total_size += str.size();
            file << str;
        }

        std::cout << "Unique words count: " << unique_words.size() << std::endl;
    } // close file before renaming

    std::stringstream ss;
    ss << "file_" << unique_words.size() << ".txt";

    std::filesystem::rename(std::filesystem::current_path() / "tmp.txt", std::filesystem::current_path() / ss.str());

    return 0;
}
