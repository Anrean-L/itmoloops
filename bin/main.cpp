#include <cstdint>
#include <iostream>
#include <vector>

#include "song_parser/song_parser.hpp"
#include "song_renderer.hpp"
#include "wav_file.hpp"
#ifndef ITMOLOOPS_NOTES_PATH
#error "ITMOLOOPS_NOTES_PATH is not defined"
#endif

namespace {

constexpr const char* kNotesPath = ITMOLOOPS_NOTES_PATH;

void PrintUsage(const char* program) {
    std::cout << "Usage: " << program << " <script.txt> <output.wav>\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Arguments error\n";
        PrintUsage(argv[0]);
        return 1;
    }
    itmoloops::FrequencyMap frequency_map(ITMOLOOPS_NOTES_PATH);
    auto composition = itmoloops::ParseComposition(argv[1], frequency_map);
    if (!composition) {
        std::cerr << "ParsingError\n";
        return 2;
    }
    std::cout << "Reading has been completed\n";

    std::vector<int16_t> samples = composition->CreateComposition();
    std::cout << "Composition has been generated\n";

    itmoloops::WavWriter(argv[2], samples);
    std::cout << "Composition has been created successfully\n";

    return 0;
}
