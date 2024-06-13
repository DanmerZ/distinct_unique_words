#include "unique_words_counter.h"

void trivial_solution(const char *file_name) {
  std::ifstream ifs(file_name);
  std::unordered_set<std::string> unique_words;

  std::string word;
  while (std::getline(ifs, word, ' ')) {
    unique_words.insert(word);
  }

  std::cout << unique_words.size() - 1 << std::endl;
}

int main([[maybe_unused]] int argc, char const *argv[]) {
  const char *file_name = argv[1];

  if (!std::filesystem::exists(file_name)) {
    std::cerr << "Could not find a file " << file_name << std::endl;
    return -1;
  }

  // trivial_solution(file_name);

  const std::uintmax_t memory_limit_in_bytes = 100'000'000ull;
  UniqueWordsCounter uwc(file_name, memory_limit_in_bytes);
  std::cout << uwc.count() << std::endl;

  return 0;
}