#include "song_parser.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "instrument_builder.hpp"
#include "song_parser/effect_builder.hpp"
#include "song_parser/utils.hpp"
#include "song_renderer.hpp"

using Param = std::pair<std::string, std::string>;

namespace itmoloops {

namespace {

enum class State {
    Global,
    Instrument,
    Pattern,
};

}  // namespace

FrequencyMap::FrequencyMap(const std::string& file_path) {
    std::ifstream in(file_path);
    std::string line;

    while (std::getline(in, line)) {
        std::vector<std::string> tokens = Split(line);
        if (tokens.size() >= 2) {
            frequency_.emplace_back(tokens[0], std::stof(tokens[1]));
        }
    }
    std::sort(frequency_.begin(), frequency_.end());
}

float FrequencyMap::GetFrequency(const std::string& note) const {
    auto it = std::lower_bound(frequency_.begin(), frequency_.end(),
                               std::make_pair(note, 0.f));
    if (it == frequency_.end()) {
        return 0;
    }
    return it->second;
}

std::unique_ptr<Composition> ParseComposition(
    const std::string& file_path, const FrequencyMap& frequency_map) {
    std::ifstream in(file_path);
    if (!in) {
        return nullptr;
    }
    auto comp = std::make_unique<Composition>();

    State state = State::Global;

    std::string current_pattern_name;
    std::unique_ptr<Pattern> current_pattern;
    std::unique_ptr<InstrumentBuilder> instrument_builder;

    std::string line;
    while (std::getline(in, line)) {
        line = Trim(StripComment(line));
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> tokens = Split(line);
        switch (state) {
            case State::Global: {
                if (tokens[0] == "bpm") {
                    comp->SetBpm(std::stoul(tokens[1]));
                } else if (tokens[0] == "instrument") {
                    if (tokens.size() < 3) {
                        return nullptr;
                    }
                    instrument_builder = std::make_unique<InstrumentBuilder>(
                        tokens[1], tokens[2]);
                    state = State::Instrument;
                } else if (tokens[0] == "pattern") {
                    if (tokens.size() < 4) {
                        return nullptr;
                    }
                    current_pattern_name = tokens[1];
                    current_pattern =
                        std::make_unique<Pattern>(std::stoul(tokens[3]));

                    state = State::Pattern;
                }
                break;
            }
            case State::Instrument: {
                if (tokens[0] == "end") {
                    std::unique_ptr<Instrument> new_instrument =
                        instrument_builder->Build(frequency_map);
                    if (!new_instrument) {
                        return nullptr;
                    }
                    comp->AddInstrument(instrument_builder->GetName(),
                                        std::move(new_instrument));
                    state = State::Global;
                } else if (tokens[0] == "effect") {
                    if (tokens.size() < 2) {
                        return nullptr;
                    }
                    EffectBuilder effect_builder(tokens[1]);
                    for (size_t i = 2; i < tokens.size(); ++i) {
                        auto [k, v] = SplitOnce(tokens[i], '=');
                        effect_builder.AddParam(k, v);
                    }
                    std::unique_ptr<Effect> effect = effect_builder.Build();
                    if (!effect) {
                        std::cout << "d";
                        return nullptr;
                    }
                    instrument_builder->AddEffect(effect);
                } else {
                    Param p = SplitOnce(line, '=');
                    instrument_builder->AddParam(Trim(p.first), Trim(p.second));
                }
                break;
            }
            case State::Pattern: {
                if (tokens[0] == "end") {
                    comp->AddPattern(current_pattern_name,
                                     std::move(current_pattern));
                    state = State::Global;
                } else {
                    uint32_t start = std::stoul(tokens[0]);
                    if (tokens.size() == 2) {
                        current_pattern->AddCall(start, tokens[1].substr(1));
                    } else if (tokens.size() >= 5) {
                        current_pattern->AddNote(
                            start,
                            {.duration = std::stoul(tokens[3]),
                             .frequency = frequency_map.GetFrequency(tokens[2]),
                             .velocity = std::stof(tokens[4]) / 100.f},
                            tokens[1]);
                    } else {
                        return nullptr;
                    }
                }
                break;
            }
        }
    }
    return comp;
}

};  // namespace itmoloops
