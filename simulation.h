//
// Created by Egor Demidov on 7/25/24.
//

#ifndef GUI_DESIGN_SOOT_DEM_SIMULATION_H
#define GUI_DESIGN_SOOT_DEM_SIMULATION_H

#include <string>
#include <filesystem>
#include <map>

#include <Eigen/Eigen>

enum ParameterType {INTEGER, REAL, STRING, PATH};

extern constexpr const char * parameter_type_to_string(ParameterType type);

extern ParameterType parameter_type_from_string(const char * string);

struct ParameterValue {
    long integer_value;
    double real_value;
    std::string string_value;
    std::filesystem::path path_value;
};

extern std::string parameter_value_to_string(ParameterType type, ParameterValue const & value);

using parameter_heap_t = std::map<std::string, std::pair<ParameterType, ParameterValue>>;

class Simulation {
public:
    explicit Simulation(parameter_heap_t const & parameter_heap);
    virtual ~Simulation() = default;
    virtual std::tuple<
    std::string,
    std::vector<Eigen::Vector3d>,
    std::vector<Eigen::Vector3d>,
    std::vector<Eigen::Vector3d>> perform_iterations() = 0;

    long get_integer_parameter(std::string const & id) const;
    double get_real_parameter(std::string const & id) const;
    std::string get_string_parameter(std::string const & id) const;
    std::filesystem::path get_path_parameter(std::string const & id) const;

protected:
    parameter_heap_t parameters;
};

#endif //GUI_DESIGN_SOOT_DEM_SIMULATION_H
