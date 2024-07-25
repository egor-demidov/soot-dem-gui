//
// Created by egor on 7/23/24.
//

#ifndef GUI_DESIGN_SOOT_DEM_AGGREGATION_H
#define GUI_DESIGN_SOOT_DEM_AGGREGATION_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <filesystem>

#include <Eigen/Eigen>

#include <libgran/hamaker_force/hamaker_force.h>
#include <libgran/contact_force/contact_force.h>
#include <libgran/granular_system/granular_system_neighbor_list.h>

#include <coating_force.h>
#include <aggregate.h>

#include <writer.h>
#include <energy.h>
#include <reader.h>
#include <aggregate_stats.h>
#include <random_engine.h>
#include <break_neck.h>

#include "simulation.h"

class AggregationSimulation : public Simulation {
public:
    using contact_force_model_t = contact_force_functor<Eigen::Vector3d, double>;
    using hamaker_force_model_t = hamaker_functor<Eigen::Vector3d, double>;
    using binary_force_container_t = binary_force_functor_container<Eigen::Vector3d, double, contact_force_model_t, hamaker_force_model_t>;
    using unary_force_container_t = unary_force_functor_container<Eigen::Vector3d, double>;
    using granular_system_t = granular_system_neighbor_list<Eigen::Vector3d, double, rotational_velocity_verlet_half,
            rotational_step_handler, binary_force_container_t, unary_force_container_t>;

    class granular_system_neighbor_list_mutable_velocity : public granular_system_t {
    public:
        using granular_system_t::granular_system_neighbor_list;

        std::vector<Eigen::Vector3d> & get_v() {
            return this->v;
        }
    };

    AggregationSimulation(
            std::ostream & output_stream,
            std::vector<Eigen::Vector3d> & x0_buffer,
            parameter_heap_t const & parameter_heap
    );

    std::tuple<
        std::string,
        std::vector<Eigen::Vector3d>> perform_iterations() override;

    // TODO: remove after debugging
    static constexpr const char * default_values[]{
            "1e-19",
            "1e-9",
            "5e-15",
            "5.6e-08",
            "1.78571e+08",
            "1e-9",
            "0.0",
            "5e-09",
            "1.25e-06",
            "2.5e-10",
            "6.25e-08",
            "2.5e-10",
            "6.25e-08",
            "1e-09",
            "2.5e-07",
            "1e-09",
            "10000",
            "1e+06",
            "10000",
            "1e+07",
            "10000",
            "1e+07",
            "10000",
            "1e+07",
            "0.1",
            "0.1",
            "1",
            "1",
            "1",
            "1",
            "1.4e-08",
            "7e-08",
            "1700",
            "10000",
            "20",
            "0",
            "mackowski",
            "aggregate.txt",
    };
    static constexpr std::tuple<const char *, ParameterType, const char *> PARAMETERS[] {
            {"A", REAL, "Hamaker constant"},
            {"d_crit", REAL, "Critical separation for a contact"},
            {"dt", REAL, "Integration time step"},
            {"f_coat_cutoff", REAL, "Capillary force cutoff distance"},
            {"f_coat_drop_rate", REAL, "Capillary force drop rate"},
            {"f_coat_max", REAL, "Capillary force maximum magnitude"},
            {"frac_necks", REAL, "Necking fraction"},
            {"gamma_n", REAL, ""},
            {"gamma_n_bond", REAL, ""},
            {"gamma_o", REAL, ""},
            {"gamma_o_bond", REAL, ""},
            {"gamma_r", REAL, ""},
            {"gamma_r_bond", REAL, ""},
            {"gamma_t", REAL, ""},
            {"gamma_t_bond", REAL, ""},
            {"h0", REAL, ""},
            {"k_n", REAL, ""},
            {"k_n_bond", REAL, ""},
            {"k_o", REAL, ""},
            {"k_o_bond", REAL, ""},
            {"k_r", REAL, ""},
            {"k_r_bond", REAL, ""},
            {"k_t", REAL, ""},
            {"k_t_bond", REAL, ""},
            {"mu_o", REAL, ""},
            {"mu_r", REAL, ""},
            {"mu_t", REAL, ""},
            {"phi_o", REAL, ""},
            {"phi_r", REAL, ""},
            {"phi_t", REAL, ""},
            {"r_part", REAL, ""},
            {"r_verlet", REAL, ""},
            {"rho", REAL, ""},
            {"dump_period", INTEGER, ""},
            {"neighbor_update_period", INTEGER, ""},
            {"rng_seed", INTEGER, ""},
            {"aggregate_type", STRING, "vtk / flage / mackowski"},
            {"aggregate_path", PATH, ""}
    };
    static constexpr size_t N_PARAMETERS = sizeof(PARAMETERS) / sizeof(PARAMETERS[0]);


private:
    double mass, inertia, r_part, dt, box_size;
    long dump_period, neighbor_update_period;
    size_t current_step = 0;
    std::unique_ptr<contact_force_model_t> contact_model;
    std::unique_ptr<hamaker_force_model_t> hamaker_model;
    std::unique_ptr<unary_force_container_t> unary_force_container;
    std::unique_ptr<binary_force_container_t> binary_force_container;
    std::unique_ptr<granular_system_neighbor_list_mutable_velocity> granular_system;
    rotational_step_handler<std::vector<Eigen::Vector3d>, Eigen::Vector3d> step_handler_instance;
};

#endif //GUI_DESIGN_SOOT_DEM_AGGREGATION_H
