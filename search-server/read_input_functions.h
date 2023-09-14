#pragma once
#include <string>
#include <vector>

std::string ReadLine();
int ReadLineWithNumber();
//std::vector<std::string> SplitIntoWords(const std::string& text);
std::vector<std::string_view> SplitIntoWords(const std::string_view str);
