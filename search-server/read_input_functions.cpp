#include "read_input_functions.h"
#include <iostream>
#include <stdint.h>


std::string ReadLine() {
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}

// std::vector<std::string_view> SplitIntoWords(const std::string_view text) {
//     std::vector<std::string_view> words;
//     std::string_view word;
//     for (const char c : text) {
//         if (c == ' ') {
//             if (!word.empty()) {
//                 words.push_back(word);
//                 word.clear();
//             }
//         } else {
//             word += c;
//         }
//     }
//     if (!word.empty()) {
//         words.push_back(word);
//     }

//     return words;
// }

std::vector<std::string_view> SplitIntoWords(const std::string_view str) {
    std::vector<std::string_view> result;
    // 1
    int64_t pos = str.find_first_not_of(" ");
    // 2
    const int64_t pos_end = str.npos;
    // 3
    while (pos != pos_end) {
        // 4
        int64_t space = str.find(' ', pos);
        // 5
        result.push_back(space == pos_end ? str.substr(pos) : str.substr(pos, space - pos));
        // 6
        pos = str.find_first_not_of(" ", space);
    }

    return result;
} 
