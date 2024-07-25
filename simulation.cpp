//
// Created by Egor Demidov on 7/25/24.
//

#include <iostream>

#include "simulation.h"

Simulation::Simulation(parameter_heap_t const & parameter_heap) : parameters{parameter_heap} {}

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
