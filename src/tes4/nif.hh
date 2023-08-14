#pragma once

#include "../engine/object.hh"

#include <vector>
#include <cinttypes>
#include <string>

namespace tes4 {

void load_nif(std::vector<uint8_t> const &, std::string const &name, engine::object &obj);

}
