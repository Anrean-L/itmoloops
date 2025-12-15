#pragma once

#include <string>

namespace itmoloops {

class AppConfig {
   public:
    const std::string& GetScriptPath() const;
    const std::string& GetOutputPath() const;

    bool ReadFromCli(int argc, char* argv[]);
    void ReadInteractively();

   private:
    std::string script_path_;
    std::string output_path_;
};

}  // namespace itmoloops
