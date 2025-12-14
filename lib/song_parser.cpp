#include "song_parser.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "song_renderer.hpp"

namespace itmoloops {

namespace {

enum class State {
    Global,
    Instrument,
    Pattern,
};

std::string Trim(const std::string& s) {
    size_t l = 0;
    while (l < s.length() && std::isspace(s[l])) {
        ++l;
    }
    size_t r = s.length() - 1;
    while (r >= 0 && std::isspace(s[r])) {
        --r;
    }
    return s.substr(l, r - l + 1);
}

std::string StripComment(const std::string& s) {
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '#' && (i == 0 || std::isspace(s[i - 1]))) {
            return s.substr(0, std::max(0ull, i - 1));
        }
    }
    return s;
}

std::vector<std::string> Split(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> res;
    for (std::string token; iss >> token;) {
        res.push_back(token);
    }
    return res;
}

std::pair<std::string, std::string> SplitOnce(const std::string& s,
                                              char separator) {
    size_t separation_idx = s.find(separator);
    return {s.substr(0, separation_idx), s.substr(separation_idx + 1)};
}

class InstrumentBuilder {
   public:
    InstrumentBuilder(const std::string& name, const std::string& type)
        : name_(name), type_(type) {}
    void AddParam(std::string key, std::string value) { params_[key] = value; }
    std::string Name() const { return name_; }

    std::unique_ptr<Instrument> Build();

   private:
    std::string name_;
    std::string type_;
    std::unordered_map<std::string, std::string> params_;
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
                        std::make_unique<Pattern>(stoi(tokens[3]));

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
