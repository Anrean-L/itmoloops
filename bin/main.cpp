#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "app_config.hpp"
#include "config.hpp"
#include "song_parser/song_parser.hpp"
#include "song_renderer.hpp"
#include "wav_file.hpp"

#ifndef ITMOLOOPS_NOTES_PATH
#error "ITMOLOOPS_NOTES_PATH is not defined"
#endif

namespace {

constexpr const char* kNotesPath = ITMOLOOPS_NOTES_PATH;

void LogInfo(const std::string& msg) {
    std::cout << " ▶ " << msg << ansi::kReset << '\n';
}

void LogError(const std::string& msg) {
    std::cerr << ansi::kRed << " ✖ " << msg << ansi::kReset << '\n';
}

void LogOk(const std::string& msg) {
    std::cout << ansi::kGreen << " ✔ " << msg << ansi::kReset << '\n';
}

void PrintBanner() {
    std::cout << ansi::kPurple << ansi::kBold
              << "╔══════════════════════════════════════════╗\n"
              << "║                ITMO LOOPS                ║\n"
              << "║         Text-based music renderer        ║\n"
              << "╚══════════════════════════════════════════╝\n"
              << ansi::kReset;
}

bool PrintAsciiArtFromScript(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(in, line)) {
        if (line.starts_with('#')) {
            lines.push_back(line.substr(1));
        } else {
            break;
        }
    }
    if (lines.empty()) {
        return true;
    }

    std::cout << '\n';
    for (const std::string& l : lines) {
        std::cout << ansi::kPurple << l << ansi::kReset << '\n';
    }
    std::cout << '\n';
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    PrintBanner();
    itmoloops::AppConfig config;

    if (argc >= 2 && !config.ReadFromCli(argc, argv)) {
        return 1;
    } else if (argc == 1) {
        config.ReadInteractively();
    }

    if (!PrintAsciiArtFromScript(config.GetScriptPath())) {
        return 2;
    }

    LogInfo("Loading notes database...");
    itmoloops::FrequencyMap frequency_map(kNotesPath);
    LogOk("Notes loaded");

    LogInfo("Parsing composition...");
    auto composition =
        itmoloops::ParseComposition(config.GetScriptPath(), frequency_map);
    if (!composition) {
        LogError("Failed to parse");
        return 3;
    }
    LogOk("Composition parsed");

    LogInfo("Rendering audio...");
    std::vector<int16_t> samples = composition->CreateComposition();
    LogOk("Audio rendered");

    LogInfo("Writing WAV file");
    itmoloops::WavWriter(config.GetOutputPath(), samples);
    LogOk("Saved to " + config.GetOutputPath());

    std::cout << '\n';
    LogOk("Done. Enjoy the music! ✨");
    return 0;
}
