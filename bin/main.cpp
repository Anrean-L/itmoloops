#include <cstdint>
#include <iostream>
#include <vector>

#include "song_parser/song_parser.hpp"
#include "song_renderer.hpp"
#include "wav_file.hpp"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Arguments error\n";
        return 1;
    }
    itmoloops::FrequencyMap frequency_map(argv[2]);
    auto composition = itmoloops::ParseComposition(argv[1], frequency_map);
    if (!composition) {
        std::cerr << "ParsingError\n";
        return 2;
    }
    std::cout << "Reading has been completed\n";

    std::vector<int16_t> samples = composition->CreateComposition();
    std::cout << "Composition has been generated\n";

    itmoloops::WavWriter(argv[3], samples);
    std::cout << "Composition has been created successfully\n";

    return 0;
}
