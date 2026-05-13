#pragma once

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "song_parser/song_parser.hpp"
#include "song_renderer.hpp"
#include "utils.hpp"

namespace itmoloops {

class InstrumentBuilder {
   public:
    InstrumentBuilder(std::string name, std::string type)
        : name_(std::move(name)), type_(std::move(type)) {}

    const std::string& GetName() const { return name_; }

    void SetHeading(std::string name, std::string type) {
        name_ = std::move(name);
        type_ = std::move(type);
    }

    void AddParam(std::string key, std::string value);
    void AddEffect(std::unique_ptr<Effect>& effect);

    std::unique_ptr<Instrument> Build(const FrequencyMap& frequency_map);

   private:
    std::string name_;
    std::string type_;
    std::vector<Param> params_;
    VectorEffects effects_;

    std::optional<std::string> GetParam(const std::string& key) const;

    float GetFloat(const std::string& key, float def) const;

    uint32_t GetUint(const std::string& key, uint32_t def) const;

    std::unique_ptr<Instrument> BuildSampler(float attack, float release,
                                             const FrequencyMap& frequency_map);

    std::unique_ptr<Instrument> BuildSquare(float attack, float release);

    std::unique_ptr<Instrument> BuildSine(float attack, float release);

    std::unique_ptr<Instrument> BuildTriangle(float attack, float release);
};

}  // namespace itmoloops
