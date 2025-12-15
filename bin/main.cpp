#include <cstdint>
#include <iostream>
#include <vector>

#include "song_parser/song_parser.hpp"
#include "song_renderer.hpp"
#include "wav_file.hpp"

int main(int argc, char* argv[]) {
    if (argc < 4) {
        return 1;
    }
    itmoloops::FrequencyMap frequency_map(argv[2]);
    auto composition = itmoloops::ParseComposition(argv[1], frequency_map);
    if (!composition) {
        return 2;
    }
    std::cout << "Reading has been completed\n";

    std::vector<int16_t> samples = composition->CreateComposition();
    std::cout << "Composition has been generated\n";

    itmoloops::WavWriter(argv[3], samples);

    return 0;
}
