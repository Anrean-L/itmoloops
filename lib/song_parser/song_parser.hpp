#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "song_renderer.hpp"

namespace itmoloops {

using Frequency = std::pair<std::string, float>;

class FrequencyMap {
   public:
    explicit FrequencyMap(const std::string& file_path);

    float GetFrequency(const std::string& note) const;

   private:
    std::vector<Frequency> frequency_;
};

std::unique_ptr<Composition> ParseComposition(
    const std::string& file_path, const FrequencyMap& Frequency_map);

};  // namespace itmoloops
