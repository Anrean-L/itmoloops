#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "song_parser/utils.hpp"
#include "song_renderer.hpp"

namespace itmoloops {

class EffectBuilder {
   public:
    explicit EffectBuilder(const std::string& type) : type_(type) {}

    void AddParam(std::string key, std::string value);

    std::unique_ptr<Effect> Build() const;

   private:
    std::string type_;
    std::vector<Param> params_;

    std::optional<float> GetFloat(const std::string& key) const;
};

}  // namespace itmoloops
