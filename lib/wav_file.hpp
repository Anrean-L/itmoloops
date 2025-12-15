#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace itmoloops {

bool WavWriter(const std::string& file_path,
               const std::vector<int16_t>& samples);

class WavReader {
   public:
    explicit WavReader(const std::string& file_path);
    bool CheckValidity() const { return is_valid_; }
    const std::vector<int16_t>& GetData() const { return data_; }

   private:
    std::vector<int16_t> data_;
    bool is_valid_ = true;
};

};  // namespace itmoloops
