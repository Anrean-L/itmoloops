#pragma once

#include <memory>
#include <string>

#include "song_renderer.hpp"

namespace itmoloops {

std::unique_ptr<Composition> ParseComposition(std::string file_path);

};  // namespace itmoloops
