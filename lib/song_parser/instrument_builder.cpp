#include "instrument_builder.hpp"

#include <memory>
#include <string>
#include <utility>

#include "song_parser/song_parser.hpp"
#include "song_renderer.hpp"

namespace itmoloops {

void InstrumentBuilder::AddParam(std::string key, std::string value) {
    for (auto& [k, v] : params_) {
        if (k == key) {
            v = std::move(value);
            return;
        }
    }
    params_.emplace_back(std::move(key), std::move(value));
}

void InstrumentBuilder::AddEffect(std::unique_ptr<Effect>& effect) {
    effects_.emplace_back(std::move(effect));
}

std::unique_ptr<Instrument> InstrumentBuilder::Build(
    const FrequencyMap& frequency_map) {
    float attack = GetFloat("attack", 0.f);
    float release = GetFloat("release", 0.f);
    if (type_ == "sampler") {
        return BuildSampler(attack, release, frequency_map);
    } else if (type_ == "square") {
        return BuildSquare(attack, release);
    } else if (type_ == "sine") {
        return BuildSine(attack, release);
    } else if (type_ == "triangle") {
        return BuildTriangle(attack, release);
    }
    return nullptr;
}

std::optional<std::string> InstrumentBuilder::GetParam(
    const std::string& key) const {
    for (auto& [k, v] : params_) {
        if (k == key) {
            return v;
        }
    }
    return std::nullopt;
}

float InstrumentBuilder::GetFloat(const std::string& key, float def) const {
    auto v = GetParam(key);
    if (v) {
        return std::stof(*v);
    }
    return def;
}

uint32_t InstrumentBuilder::GetUint(const std::string& key,
                                    uint32_t def) const {
    auto v = GetParam(key);
    if (v) {
        return std::stoul(*v);
    }
    return def;
}

std::unique_ptr<Instrument> InstrumentBuilder::BuildSampler(
    float attack, float release, const FrequencyMap& frequency_map) {
    auto sample = GetParam("sample");
    auto root_note = GetParam("root");
    if (!sample || !root_note) {
        return nullptr;
    }
    float root = frequency_map.GetFrequency(*root_note);
    uint32_t loop_start = 0;
    uint32_t loop_end = 0;
    auto loop = GetParam("loop");
    if (loop) {
        auto [first, second] = SplitOnce(*loop, ',');
        loop_start = std::stoul(first);
        loop_end = std::stoul(second);
    }
    return std::make_unique<SamplerInstrument>(std::move(*sample), root,
                                               loop_start, loop_end, attack,
                                               release, std::move(effects_));
}

std::unique_ptr<Instrument> InstrumentBuilder::BuildSquare(float attack,
                                                           float release) {
    uint32_t duty = GetUint("duty", 50);
    return std::make_unique<SquareInstrument>(duty, attack, release,
                                              std::move(effects_));
}

std::unique_ptr<Instrument> InstrumentBuilder::BuildSine(float attack,
                                                         float release) {
    return std::make_unique<SineInstrument>(attack, release,
                                            std::move(effects_));
}

std::unique_ptr<Instrument> InstrumentBuilder::BuildTriangle(float attack,
                                                             float release) {
    return std::make_unique<TriangleInstrument>(attack, release,
                                                std::move(effects_));
}

}  // namespace itmoloops
