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

#include <iostream>

#include "simulation.h"

ParameterType parameter_type_from_string(const char * string) {
    if (strcmp(string, "integer") == 0) {
        return INTEGER;
    } else if (strcmp(string, "real") == 0) {
        return REAL;
    } else if (strcmp(string, "string") == 0) {
        return STRING;
    } else if (strcmp(string, "path") == 0) {
        return PATH;
    }
}

std::string parameter_value_to_string(ParameterType type, ParameterValue const & value) {
    std::stringstream ss;
    switch (type) {
        case INTEGER:
            ss << value.integer_value;
            return ss.str();
        case REAL:
            ss << value.real_value;
            return ss.str();
        case STRING:
            return value.string_value;
        case PATH:
            return value.path_value.string();
    }
}

Simulation::Simulation(parameter_heap_t const & parameter_heap,
                       std::filesystem::path const & working_directory)
                        : parameters{parameter_heap}
                        , simulation_working_directory{working_directory} {

    if (!std::filesystem::is_directory(working_directory / "run") || !std::filesystem::is_directory(working_directory / "run")) {
        std::filesystem::create_directory(working_directory / "run"); // create src folder
    }
}

long Simulation::get_integer_parameter(std::string const & id) const {
    auto parameter_itr = parameters.find(id);
    if (parameter_itr == parameters.end() || parameter_itr->second.first != INTEGER) {
        std::cerr << "Required integer parameter `" << id << "` is missing" << std::endl;
        exit(EXIT_FAILURE);
    }
    return parameter_itr->second.second.integer_value;
}

double Simulation::get_real_parameter(std::string const & id) const {
    auto parameter_itr = parameters.find(id);
    if (parameter_itr == parameters.end() || parameter_itr->second.first != REAL) {
        std::cerr << "Required real parameter `" << id << "` is missing" << std::endl;
        exit(EXIT_FAILURE);
    }
    return parameter_itr->second.second.real_value;
}

std::string Simulation::get_string_parameter(std::string const & id) const {
    auto parameter_itr = parameters.find(id);
    if (parameter_itr == parameters.end() || parameter_itr->second.first != STRING) {
        std::cerr << "Required string parameter `" << id << "` is missing" << std::endl;
        exit(EXIT_FAILURE);
    }
    return parameter_itr->second.second.string_value;
}

std::filesystem::path Simulation::get_path_parameter(std::string const & id) const {
    auto parameter_itr = parameters.find(id);
    if (parameter_itr == parameters.end() || parameter_itr->second.first != PATH) {
        std::cerr << "Required path parameter `" << id << "` is missing" << std::endl;
        exit(EXIT_FAILURE);
    }
    return parameter_itr->second.second.path_value;
}
