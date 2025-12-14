#include "utils.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace itmoloops {

std::string Trim(const std::string& s) {
    size_t l = 0;
    while (l < s.length() && std::isspace(s[l])) {
        ++l;
    }
    size_t r = s.length() - 1;
    while (r >= 0 && std::isspace(s[r])) {
        --r;
    }
    return s.substr(l, r - l + 1);
}

std::string StripComment(const std::string& s) {
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '#' && (i == 0 || std::isspace(s[i - 1]))) {
            return s.substr(0, std::max(0ull, i - 1));
        }
    }
    return s;
}

std::vector<std::string> Split(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> res;
    for (std::string token; iss >> token;) {
        res.push_back(token);
    }
    return res;
}

Param SplitOnce(const std::string& s, char separator) {
    size_t separation_idx = s.find(separator);
    return {s.substr(0, separation_idx), s.substr(separation_idx + 1)};
}

}  // namespace itmoloops