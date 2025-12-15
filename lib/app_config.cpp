#include "app_config.hpp"

#include <iostream>
#include <string>

#include "config.hpp"

namespace itmoloops {

namespace {

void PrintUsage(const char* program) {
    std::cout << "Usage: " << program << " <script.txt> <output.wav>\n";
}

std::string ReplaceExtension(std::string path, std::string_view new_ext) {
    size_t dot = path.find_last_of('.');
    size_t slash = path.find_last_of("/\\");

    if (dot != std::string::npos &&
        (slash == std::string::npos || dot > slash)) {
        path.erase(dot);
    }

    return path + std::string(new_ext);
}

}  // namespace

bool AppConfig::ReadFromCli(int argc, char* argv[]) {
    if (argc < 2 || argv[1] == kHelpFlag) {
        PrintUsage(argv[0]);
        return false;
    }
    script_path_ = argv[1];
    output_path_ =
        ReplaceExtension(argc >= 3 ? argv[2] : script_path_, kWavExtension);
    return true;
}

void AppConfig::ReadInteractively() {
    std::cout << "Enter path to script file: ";
    std::getline(std::cin, script_path_);
    std::cout << "Enter output WAV file (empty = auto): ";
    std::getline(std::cin, output_path_);

    if (output_path_.empty()) {
        output_path_ = ReplaceExtension(script_path_, kWavExtension);
    }
}

const std::string& AppConfig::GetScriptPath() const { return script_path_; }

const std::string& AppConfig::GetOutputPath() const { return output_path_; }

}  // namespace itmoloops
