#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace itmoloops {

class WavReader {
   public:
    explicit WavReader(std::string file_path);
    bool CheckValidity() const { return is_valid_; }
    const std::vector<int16_t>& GetData() const { return data_; }

   private:
    std::vector<int16_t> data_;
    bool is_valid_ = true;
};

};  // namespace itmoloops
