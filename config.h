// Copyright (C) 2024 Egor Demidov
// This file is part of soot-dem-gui
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef GUI_DESIGN_SOOT_DEM_CONFIG_H
#define GUI_DESIGN_SOOT_DEM_CONFIG_H

#include <filesystem>
#include <string>
#include <tuple>

#include "exceptions.h"
#include "simulation.h"


std::tuple<std::string, parameter_heap_t> load_config_file(std::filesystem::path const & config_path);

void write_config_file(std::filesystem::path const & config_path,
                       const char * simulation_config_signature,
                       parameter_heap_t const & parameters);


#endif //GUI_DESIGN_SOOT_DEM_CONFIG_H
