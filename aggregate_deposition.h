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

#ifndef GUI_DESIGN_SOOT_DEM_AGGREGATE_DEPOSITION_H
#define GUI_DESIGN_SOOT_DEM_AGGREGATE_DEPOSITION_H

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>
#include <iomanip>
#include <filesystem>

#include <Eigen/Eigen>

#include <libgran/hamaker_force/hamaker_force.h>
#include <libgran/granular_system/granular_system_neighbor_list.h>

#include <coating_force.h>
#include <rect_substrate.h>
#include <aggregate.h>

#include <writer.h>
#include <energy.h>
#include <reader.h>
#include <aggregate_stats.h>
#include <random_engine.h>
#include <break_neck.h>

#include "simulation.h"

class AggregateDepositionSimulation : public Simulation {
public:
    using aggregate_model_t = aggregate<Eigen::Vector3d, double>;
    using rect_substrate_model_t = rect_substrate<Eigen::Vector3d, double>;
    using binary_force_container_t = binary_force_functor_container<Eigen::Vector3d, double, aggregate_model_t>;
    using unary_force_container_t = unary_force_functor_container<Eigen::Vector3d, double, rect_substrate_model_t>;
    using granular_system_t = granular_system_neighbor_list<Eigen::Vector3d, double, rotational_velocity_verlet_half,
            rotational_step_handler, binary_force_container_t, unary_force_container_t>;

    explicit AggregateDepositionSimulation(
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

    static constexpr const char * config_file_signature = "gui_deposition";
    static constexpr const char * combo_label = "Deposition";
    static constexpr unsigned int combo_id = 2;

    // TODO: remove after debugging
    static constexpr const char * default_values[]{
            "1e-19",
            "1e-19",
            "1e-9",
            "5e-14",
            "5e-09",
            "1.25e-07",
            "5e-09",
            "2.5e-10",
            "6.25e-09",
            "2.5e-10",
            "2.5e-10",
            "6.25e-09",
            "2.5e-10",
            "1e-09",
            "2.5e-08",
            "1e-09",
            "1e-09",
            "1e-09",
            "10000",
            "100000",
            "10000",
            "10000",
            "100000",
            "10000",
            "10000",
            "100000",
            "10000",
            "10000",
            "100000",
            "10000",
            "0.1",
            "0.1",
            "0.1",
            "0.1",
            "1",
            "1",
            "1",
            "1",
            "1",
            "1",
            "1",
            "1",
            "1.4e-08",
            "7e-08",
            "1700",
            "0",
            "0",
            "0",
            "1",
            "9e-07",
            "10000",
            "20",
            "mackowski",
            "aggregate.txt",
    };

    static constexpr const char * DESCRIPTION = "An aggregate is loaded\n"
                                                "and necks are inserted between\n"
                                                "neighboring primary particles.\n"
                                                "A prescribed fraction of necks\n"
                                                "is broken randomly and capillary\n"
                                                "force is applied to primary particles\n"
                                                "to induce restructuring\n\n"
                                                "This simulates restructuring\n"
                                                "of thinly-coated soot aggregates";

    static constexpr std::tuple<const char *, ParameterType, const char *> PARAMETERS[] {
            {"A", REAL, "Hamaker constant"},
            {"A_substrate", REAL, "Substrate Hamaker constant"},
            {"d_crit", REAL, "Critical separation for a contact"},
            {"dt", REAL, "Integration time step"},
            {"gamma_n", REAL, "Normal damping coefficient"},
            {"gamma_n_bond", REAL, "Normal bond damping coefficient"},
            {"gamma_n_substrate", REAL, "Normal substrate damping coefficient"},
            {"gamma_o", REAL, "Torsional damping coefficient"},
            {"gamma_o_bond", REAL, "Torsional bond damping coefficient"},
            {"gamma_o_substrate", REAL, "Torsional substrate damping coefficient"},
            {"gamma_r", REAL, "Rotational damping coefficient"},
            {"gamma_r_bond", REAL, "Rolling bond damping coefficient"},
            {"gamma_r_substrate", REAL, "Rolling substrate damping coefficient"},
            {"gamma_t", REAL, "Tangential damping coefficient"},
            {"gamma_t_bond", REAL, "Tangential bond damping coefficient"},
            {"gamma_t_substrate", REAL, "Tangential substrate damping coefficient"},
            {"h0", REAL, "Hamaker saturation distance"},
            {"h0_substrate", REAL, "Substrate Hamaker saturation distance"},
            {"k_n", REAL, "Normal stiffness coefficient"},
            {"k_n_bond", REAL, "Normal bond stiffness coefficient"},
            {"k_n_substrate", REAL, "Normal substrate stiffness coefficient"},
            {"k_o", REAL, "Torsional stiffness coefficient"},
            {"k_o_bond", REAL, "Torsional bond stiffness coefficient"},
            {"k_o_substrate", REAL, "Torsional substrate stiffness coefficient"},
            {"k_r", REAL, "Rolling stiffness coefficient"},
            {"k_r_bond", REAL, "Rolling bond stiffness coefficient"},
            {"k_r_substrate", REAL, "Rolling substrate stiffness coefficient"},
            {"k_t", REAL, "Tangential stiffness coefficient"},
            {"k_t_bond", REAL, "Tangential bond stiffness coefficient"},
            {"k_t_substrate", REAL, "Tangential substrate stiffness coefficient"},
            {"mu_o", REAL, "Torsional friction coefficient"},
            {"mu_o_substrate", REAL, "Torsional substrate friction coefficient"},
            {"mu_r", REAL, "Rolling friction coefficient"},
            {"mu_r_substrate", REAL, "Rolling substrate friction coefficient"},
            {"mu_t", REAL, "Tangential friction coefficient"},
            {"mu_t_substrate", REAL, "Tangential substrate friction coefficient"},
            {"phi_o", REAL, "Torsional dynamic to static friction ratio"},
            {"phi_o_substrate", REAL, "Torsional substrate dynamic to static friction ratio"},
            {"phi_r", REAL, "Rolling dynamic to static friction ratio"},
            {"phi_r_substrate", REAL, "Rolling substrate dynamic to static friction ratio"},
            {"phi_t", REAL, "Tangential dynamic to static friction ratio"},
            {"phi_t_substrate", REAL, "Tangential substrate dynamic to static friction ratio"},
            {"r_part", REAL, "Primary particle radius"},
            {"r_verlet", REAL, "Verlet radius"},
            {"rho", REAL, "Density"},
            {"rot_x", REAL, "Rotate aggregate about x-axis"},
            {"rot_y", REAL, "Rotate aggregate about y-axis"},
            {"rot_z", REAL, "Rotate aggregate about z-axis"},
            {"vz0", REAL, "Initial downward velocity of the aggregate"},
            {"substrate_size", REAL, "Size of the substrate"},
            {"dump_period", INTEGER, "Dump period"},
            {"neighbor_update_period", INTEGER, "Neighbor list update period"},
            {"aggregate_type", STRING, "vtk / flage / mackowski"},
            {"aggregate_path", PATH, "Path to the aggregate file"},
    };
    static constexpr size_t N_PARAMETERS = sizeof(PARAMETERS) / sizeof(PARAMETERS[0]);
//    static constexpr const char * default_values[N_PARAMETERS]{0};

    std::tuple<std::vector<Eigen::Vector3d>, std::vector<Eigen::Vector3d>> get_neck_information() const;


private:
    double mass, inertia, r_part, dt;
    long dump_period, neighbor_update_period;
    size_t current_step = 0;
    std::unique_ptr<rect_substrate_model_t> substrate_model;
    std::unique_ptr<aggregate_model_t> aggregate_model;
    std::unique_ptr<unary_force_container_t> unary_force_container;
    std::unique_ptr<binary_force_container_t> binary_force_container;
    std::unique_ptr<granular_system_t> granular_system;
    rotational_step_handler<std::vector<Eigen::Vector3d>, Eigen::Vector3d> step_handler_instance;
};

#endif //GUI_DESIGN_SOOT_DEM_AGGREGATE_DEPOSITION_H
