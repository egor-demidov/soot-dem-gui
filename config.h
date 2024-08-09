//
// Created by Egor on 8/8/2024.
//

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
