#include "wav_file.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <string>
#include <type_traits>
#include <vector>

#include "config.hpp"

namespace itmoloops {

namespace {

constexpr size_t kWavBufferSize = 4;
constexpr uint32_t kFormatHeaderSize = 16;
constexpr char kFileId[] = "RIFF";
constexpr char kFileFormatId[] = "WAVE";
constexpr char kDataChunkId[] = "data";
constexpr char kFormatChunkId[] = "fmt ";
constexpr uint16_t kAudioFormat = 1;
constexpr uint16_t kBitsPerSample = 16;
constexpr uint16_t kChannels = 1;
constexpr uint32_t kHeaderSize = 36;

struct RiffHeader {
    char id[4];
    uint32_t size;
    char format[4];
};

struct FormatChunkHeader {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t frequency;
    uint32_t byte_per_sec;
    uint16_t byte_per_block;
    uint16_t bits_per_sample;
};

template <typename T>
T BytesToInt(const char* data) {
    static_assert(std::is_integral_v<T>, "Only integers allowed");
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i) {
        value |= (uint8_t)data[i] << (i * kBitsInByte);
    }
    return value;
}

bool CheckFormat(std::ifstream& f) {
    FormatChunkHeader chunk{};
    char buffer[kWavBufferSize];
    f.read(buffer, sizeof(chunk.audio_format));
    chunk.audio_format = BytesToInt<uint16_t>(buffer);
    f.read(buffer, sizeof(chunk.num_channels));
    chunk.num_channels = BytesToInt<uint16_t>(buffer);
    f.read(buffer, sizeof(chunk.frequency));
    chunk.frequency = BytesToInt<uint32_t>(buffer);
    f.read(buffer, sizeof(chunk.byte_per_sec));
    chunk.byte_per_sec = BytesToInt<uint32_t>(buffer);
    f.read(buffer, sizeof(chunk.byte_per_block));
    chunk.byte_per_block = BytesToInt<uint16_t>(buffer);
    f.read(buffer, sizeof(chunk.bits_per_sample));
    chunk.bits_per_sample = BytesToInt<uint16_t>(buffer);

    if (chunk.audio_format != kAudioFormat || chunk.num_channels != kChannels ||
        chunk.frequency != kFrequency ||
        chunk.bits_per_sample != kBitsPerSample ||
        chunk.byte_per_block != kChannels * kBitsPerSample / kBitsInByte ||
        chunk.byte_per_sec != kFrequency * chunk.byte_per_block) {
        return false;
    }
    return true;
}

void WriteFormatHeader(std::ofstream& out) {
    uint16_t bytes_per_block = kChannels * kBitsPerSample / kBitsInByte;
    uint32_t bytes_per_sec = kFrequency * bytes_per_block;
    out.write(kFormatChunkId, std::strlen(kFormatChunkId));
    out.write((char*)&kFormatHeaderSize, sizeof(kFormatHeaderSize));
    out.write((char*)&kAudioFormat, sizeof(kAudioFormat));
    out.write((char*)&kChannels, sizeof(kChannels));
    out.write((char*)&kFrequency, sizeof(kFrequency));
    out.write((char*)&bytes_per_sec, sizeof(bytes_per_sec));
    out.write((char*)&bytes_per_block, sizeof(bytes_per_block));
    out.write((char*)&kBitsPerSample, sizeof(kBitsPerSample));
}

}  // namespace

WavReader::WavReader(const std::string& file_path) {
    std::ifstream f(file_path, std::ios::binary);
    char buffer[kWavBufferSize];
    RiffHeader riff_header{};
    f.read(riff_header.id, sizeof(riff_header.id));
    f.read(buffer, sizeof(riff_header.size));
    riff_header.size = BytesToInt<uint32_t>(buffer);
    f.read(riff_header.format, sizeof(riff_header.format));
    if (memcmp(riff_header.id, kFileId, sizeof(riff_header.id)) != 0 ||
        memcmp(riff_header.format, kFileFormatId, sizeof(riff_header.format)) !=
            0) {
        is_valid_ = false;
        return;
    }
    char chunk_id[kWavBufferSize];
    uint32_t chunk_size = 0;
    while (true && f) {
        f.read(chunk_id, sizeof(chunk_id));
        f.read(buffer, sizeof(chunk_size));
        chunk_size = BytesToInt<uint32_t>(buffer);
        if (memcmp(chunk_id, kDataChunkId, sizeof(chunk_id)) == 0) {
            data_.resize(chunk_size / 2);
            f.read((char*)data_.data(), chunk_size);
            break;
        } else if (memcmp(chunk_id, kFormatChunkId, sizeof(chunk_id)) == 0) {
            if (chunk_size != kFormatHeaderSize || !CheckFormat(f)) {
                is_valid_ = false;
                return;
            }
        } else {
            f.seekg(chunk_size, std::ios::cur);
        }
    }
}

bool WavWriter(const std::string& file_path,
               const std::vector<int16_t>& samples) {
    std::ofstream out(file_path, std::ios::binary);
    if (!out) {
        return false;
    }
    uint32_t data_size = samples.size() * sizeof(samples[0]);
    uint32_t file_size = data_size + kHeaderSize;
    out.write(kFileId, std::strlen(kFileId));
    out.write((char*)&file_size, sizeof(file_size));
    out.write(kFileFormatId, std::strlen(kFileFormatId));

    WriteFormatHeader(out);

    out.write(kDataChunkId, std::strlen(kDataChunkId));
    out.write((char*)&data_size, sizeof(data_size));
    out.write((char*)samples.data(), data_size);

    return true;
}

}  // namespace itmoloops
