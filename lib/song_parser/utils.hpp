#pragma once

#include <string>
#include <utility>
#include <vector>

namespace itmoloops {

using Param = std::pair<std::string, std::string>;

std::string Trim(const std::string& s);

std::string StripComment(const std::string& s);

std::vector<std::string> Split(const std::string& s);

Param SplitOnce(const std::string& s, char separator);

}  // namespace itmoloops
