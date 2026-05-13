#include "effect_builder.hpp"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "song_renderer.hpp"

namespace itmoloops {

void EffectBuilder::AddParam(std::string key, std::string value) {
    for (auto& [k, v] : params_) {
        if (k == key) {
            v = std::move(value);
            return;
        }
    }
    params_.emplace_back(std::move(key), std::move(value));
}

std::unique_ptr<Effect> EffectBuilder::Build() const {
    if (type_ == "gain") {
        auto gain = GetFloat("gain");
        if (gain) {
            return std::make_unique<GainEffect>(*gain);
        }
    } else if (type_ == "tremolo") {
        auto freq = GetFloat("freq");
        auto depth = GetFloat("depth");
        if (freq && depth) {
            return std::make_unique<TremoloEffect>(*freq, *depth);
        }
    } else if (type_ == "echo") {
        auto delay = GetFloat("delay");
        auto decay = GetFloat("decay");
        if (delay && decay) {
            return std::make_unique<EchoEffect>(*delay, *decay);
        }
    }
    return nullptr;
}

std::optional<float> EffectBuilder::GetFloat(const std::string& key) const {
    for (auto& [k, v] : params_) {
        if (k == key) {
            return std::stof(v);
        }
    }
    return std::nullopt;
}

}  // namespace itmoloops
