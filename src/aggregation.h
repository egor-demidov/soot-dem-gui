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

#ifndef GUI_DESIGN_SOOT_DEM_AGGREGATION_H
#define GUI_DESIGN_SOOT_DEM_AGGREGATION_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <filesystem>

#include "deps/eigen/Eigen/Eigen"

#include "deps/libgran/include/libgran/hamaker_force/hamaker_force.h"
#include "deps/libgran/include/libgran/contact_force/contact_force.h"
#include "deps/libgran/include/libgran/granular_system/granular_system_neighbor_list.h"

#include "deps/soot-dem/src/coating_force.h"
#include "deps/soot-dem/src/aggregate.h"

#include "deps/soot-dem/src/writer.h"
#include "deps/soot-dem/src/energy.h"
#include "deps/soot-dem/src/reader.h"
#include "deps/soot-dem/src/aggregate_stats.h"
#include "deps/soot-dem/src/random_engine.h"
#include "deps/soot-dem/src/break_neck.h"

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

    explicit AggregationSimulation(
            parameter_heap_t const & parameter_heap,
            std::filesystem::path const & working_directory
    );

    bool initialize(std::ostream & output_stream,
                    std::vector<Eigen::Vector3d> & x0_buffer,
                    std::vector<Eigen::Vector3d> & neck_positions_buffer,
                    std::vector<Eigen::Vector3d> & neck_orientations_buffer) override;

    std::tuple<
        std::string,
        std::vector<Eigen::Vector3d>,
        std::vector<Eigen::Vector3d>,
        std::vector<Eigen::Vector3d>> perform_iterations() override;

    static constexpr const char * config_file_signature = "gui_aggregation";
    static constexpr const char * combo_label = "Aggregation";
    static constexpr unsigned int combo_id = 1;

    // TODO: remove after debugging
//    static constexpr const char * default_values[]{
//            "1e-19",
//            "9.90975e-07",
//            "1e-09",
//            "5e-13",
//            "5e-09",
//            "2.5e-10",
//            "2.5e-10",
//            "1e-09",
//            "1e-09",
//            "10000",
//            "10000",
//            "10000",
//            "10000",
//            "0.1",
//            "0.1",
//            "1",
//            "1",
//            "1",
//            "1",
//            "1.4e-08",
//            "7e-08",
//            "1700",
//            "1",
//            "15000",
//            "500",
//            "20",
//            "0"
//    };

    static constexpr const char * DESCRIPTION = "A box of fixed size is filled with\n"
                                                "primary particles and initial velocities\n"
                                                "with random directions are assigned to\n"
                                                "all particles. Collisions with box walls\n"
                                                "are fully elastic.\n\n"
                                                "This simulates a diffusion-limited\n"
                                                "aggregation process.";

    static constexpr std::tuple<const char *, ParameterType, const char *> PARAMETERS[] {
            {"A", REAL, "Hamaker constant"},
            {"box_size", REAL, "Simulation box size"},
            {"d_crit", REAL, "Critical separation for a contact"},
            {"dt", REAL, "Integration time step"},
            {"gamma_n", REAL, "Normal damping coefficient"},
            {"gamma_o", REAL, "Torsional damping coefficient"},
            {"gamma_r", REAL, "Rotational damping coefficient"},
            {"gamma_t", REAL, "Tangential damping coefficient"},
            {"h0", REAL, "Hamaker saturation distance"},
            {"k_n", REAL, "Normal stiffness coefficient"},
            {"k_o", REAL, "Torsional stiffness coefficient"},
            {"k_r", REAL, "Rolling stiffness coefficient"},
            {"k_t", REAL, "Tangential stiffness coefficient"},
            {"mu_o", REAL, "Torsional friction coefficient"},
            {"mu_r", REAL, "Rolling friction coefficient"},
            {"mu_t", REAL, "Tangential friction coefficient"},
            {"phi_o", REAL, "Torsional dynamic to static friction ratio"},
            {"phi_r", REAL, "Rolling dynamic to static friction ratio"},
            {"phi_t", REAL, "Tangential dynamic to static friction ratio"},
            {"r_part", REAL, "Primary particle radius"},
            {"r_verlet", REAL, "Verlet radius"},
            {"rho", REAL, "Density"},
            {"v0_part", REAL, "Initial velocity of particles"},
            {"dump_period", INTEGER, "Dump period"},
            {"n_part", INTEGER, "Number of particles"},
            {"neighbor_update_period", INTEGER, "Neighbor list update period"},
            {"rng_seed", INTEGER, "Random number generator seed"},
    };
    static constexpr size_t N_PARAMETERS = sizeof(PARAMETERS) / sizeof(PARAMETERS[0]);
    static constexpr const char * default_values[N_PARAMETERS]{0};


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
