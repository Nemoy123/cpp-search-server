#pragma once


template <typename StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string, std::less<>> non_empty_strings;
    for (const std::string_view str : strings) {
        if (!str.empty() || str.size() == 0) {
            std::string temp{str};
            non_empty_strings.insert(temp);
        }
    }
    return non_empty_strings;
}