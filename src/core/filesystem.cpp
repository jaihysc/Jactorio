// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "core/filesystem.h"

#include <fstream>
#include <sstream>
#include <string>

using namespace jactorio;

std::string core::ReadFile(const std::string& path) {
    const std::ifstream in(path);

    std::stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}
