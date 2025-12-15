#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace itmoloops {

constexpr size_t kSecondsInMinute = 60;
constexpr size_t kBitsInByte = 8;
constexpr uint32_t kFrequency = 44100;
constexpr std::string_view kRootPattern = "main";
constexpr std::string_view kHelpFlag = "--help";
constexpr std::string_view kWavExtension = ".wav";

}  // namespace itmoloops

namespace ansi {

constexpr const char* kReset = "\033[0m";
constexpr const char* kBold = "\033[1m";

constexpr const char* kPurple = "\033[35m";
constexpr const char* kCyan = "\033[36m";
constexpr const char* kGreen = "\033[32m";
constexpr const char* kRed = "\033[31m";
constexpr const char* kGray = "\033[90m";

}  // namespace ansi
