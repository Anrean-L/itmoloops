#include "song_parser.hpp"

#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "instrument_builder.hpp"
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

std::unique_ptr<Composition> ParseComposition(std::string file_path) {
    std::ifstream in(file_path);
    if (!in) {
        return nullptr;
    }
    auto comp = std::make_unique<Composition>();

    State state = State::Global;

    std::string line;
    while (std::getline(in, line)) {
        line = Trim(StripComment(line));
        if (line.empty()) {
            continue;
        }
        std::vector<std::string> tokens = Split(line);
        std::string current_pattern_name;
        std::unique_ptr<Pattern> current_pattern;
        switch (state) {
            case State::Global: {
                if (tokens[0] == "bpm") {
                    comp->SetBpm(std::stoi(tokens[1]));
                } else if (tokens[0] == "instrument") {
                    if (tokens.size() < 3) {
                        return nullptr;
                    }
                    InstrumentBuilder instrument_builder(tokens[1], tokens[2]);
                    state = State::Instrument;
                } else if (tokens[0] == "pattern") {
                    if (tokens.size() < 4) {
                        return nullptr;
                    }
                    current_pattern_name = tokens[1];
                    current_pattern =
                        std::make_unique<Pattern>(std::stoi(tokens[3]));

                    state = State::Pattern;
                }
                break;
            }
            case State::Instrument: {
                break;
            }
            case State::Pattern: {
                break;
            }
        }
    }
    return comp;
}
};  // namespace itmoloops
