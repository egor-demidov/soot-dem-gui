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

#include <iomanip>
#include "format_wrapper.h"

#include "restructuring_fixed_fraction.h"

RestructuringFixedFractionSimulation::RestructuringFixedFractionSimulation(
        parameter_heap_t const & parameter_heap,
        std::filesystem::path const & working_directory
) : Simulation(parameter_heap, working_directory) {}

bool
RestructuringFixedFractionSimulation::initialize(std::ostream &output_stream, std::vector<Eigen::Vector3d> &x0_buffer,
                                                 std::vector<Eigen::Vector3d> &neck_positions_buffer,
                                                 std::vector<Eigen::Vector3d> &neck_orientations_buffer,
                                                 std::vector<std::vector<Eigen::Vector3d>> & polygons) {
    auto rng_seed = get_integer_parameter("rng_seed");

    // General parameters
    auto rho = get_real_parameter("rho");
    auto r_verlet = get_real_parameter("r_verlet");

    // Parameters for the contact model
    auto k_n = get_real_parameter("k_n");
    auto gamma_n = get_real_parameter("gamma_n");
    auto k_t = get_real_parameter("k_t");
    auto gamma_t = get_real_parameter("gamma_t");
    auto mu_t = get_real_parameter("mu_t");
    auto phi_t = get_real_parameter("phi_t");
    auto k_r = get_real_parameter("k_r");
    auto gamma_r = get_real_parameter("gamma_r");
    auto mu_r = get_real_parameter("mu_r");
    auto phi_r = get_real_parameter("phi_r");
    auto k_o = get_real_parameter("k_o");
    auto gamma_o = get_real_parameter("gamma_o");
    auto mu_o = get_real_parameter("mu_o");
    auto phi_o = get_real_parameter("phi_o");

    // Parameters for the bonded contact model
    auto k_n_bond = get_real_parameter("k_n_bond");
    auto gamma_n_bond = get_real_parameter("gamma_n_bond");
    auto k_t_bond = get_real_parameter("k_t_bond");
    auto gamma_t_bond = get_real_parameter("gamma_t_bond");
    auto k_r_bond = get_real_parameter("k_r_bond");
    auto gamma_r_bond = get_real_parameter("gamma_r_bond");
    auto k_o_bond = get_real_parameter("k_o_bond");
    auto gamma_o_bond = get_real_parameter("gamma_o_bond");
    auto d_crit = get_real_parameter("d_crit"); // Critical separation

    // Parameters for the Van der Waals model
    auto A = get_real_parameter("A");
    auto h0 = get_real_parameter("h0");

    // Parameters for the coating model
    auto f_coat_mag = get_real_parameter("f_coat_max");
    auto f_coat_cutoff = get_real_parameter("f_coat_cutoff");
    auto f_coat_drop_rate = get_real_parameter("f_coat_drop_rate");

    auto aggregate_type = get_string_parameter("aggregate_type");
    auto aggregate_path = get_path_parameter("aggregate_path");
    auto frac_necks = get_real_parameter("frac_necks");

    // Initialization of member variables
    dt = get_real_parameter("dt");
    dump_period = get_integer_parameter("dump_period");
    neighbor_update_period = get_integer_parameter("neighbor_update_period");
    r_part = get_real_parameter("r_part");
    mass = 4.0 / 3.0 * M_PI * pow(r_part, 3.0) * rho;
    inertia = 2.0 / 5.0 * mass * pow(r_part, 2.0);

    // Declare the initial condition buffers
    std::vector<Eigen::Vector3d> x0, v0, theta0, omega0;

    if (aggregate_type == "vtk") {
        x0 = load_vtk_aggregate(simulation_working_directory / aggregate_path, r_part);
    } else if (aggregate_type == "mackowski") {
        x0 = load_mackowski_aggregate(simulation_working_directory / aggregate_path, r_part);
    } else if (aggregate_type == "flage") {
        x0 = load_flage_aggregate(simulation_working_directory / aggregate_path, r_part);
    } else {
        std::cerr << "Unrecognized aggregate type: " << aggregate_type << std::endl;
        return false;
    }

    if (x0.empty()) {
        std::cerr << "Loaded an empty aggregate" << std::endl;
        return false;
    }
    output_stream << "Loaded an aggregate of size " << x0.size() << std::endl;

    x0_buffer = x0;

    // Fill the remaining buffers with zeros
    v0.resize(x0.size());
    theta0.resize(x0.size());
    omega0.resize(x0.size());
    std::fill(v0.begin(), v0.end(), Eigen::Vector3d::Zero());
    std::fill(theta0.begin(), theta0.end(), Eigen::Vector3d::Zero());
    std::fill(omega0.begin(), omega0.end(), Eigen::Vector3d::Zero());

    aggregate_model = std::make_unique<aggregate_model_t>(
            k_n, gamma_n,
            k_t, gamma_t, mu_t, phi_t,
            k_r, gamma_r, mu_r, phi_r,
            k_o, gamma_o, mu_o, phi_o,
            k_n_bond, gamma_n_bond,
            k_t_bond, gamma_t_bond,
            k_r_bond, gamma_r_bond,
            k_o_bond, gamma_o_bond,
            d_crit, A, h0, x0, x0.size(),
            r_part, mass, inertia, dt, Eigen::Vector3d::Zero(), 0.0);

    coating_model = std::make_unique<coating_model_t>(f_coat_cutoff, f_coat_mag, f_coat_drop_rate, mass, Eigen::Vector3d::Zero());

    unary_force_container = std::make_unique<unary_force_container_t>();

    binary_force_container = std::make_unique<binary_force_container_t>(*aggregate_model, *coating_model);

    granular_system = std::make_unique<granular_system_t>(x0.size(), r_verlet, x0,
                                                          v0, theta0, omega0, 0.0, Eigen::Vector3d::Zero(), 0.0,
                                                          step_handler_instance, *binary_force_container, *unary_force_container);

    seed_random_engine(rng_seed);

    // Count the number of necks
    size_t n_necks = std::count(aggregate_model->get_bonded_contacts().begin(),
                                aggregate_model->get_bonded_contacts().end(), true) / 2;

    auto target_n_necks = size_t(double(n_necks) * frac_necks);

    output_stream << "Breaking " << n_necks - target_n_necks << " necks out of " << n_necks << std::endl;

    for (size_t i = n_necks; i > target_n_necks; i --) {
        break_random_neck(aggregate_model->get_bonded_contacts(), x0.size());
    }

    auto [neck_positions, neck_orientations] = get_neck_information();
    neck_positions_buffer = neck_positions;
    neck_orientations_buffer = neck_orientations;

    output_stream << "Dump\tTime\tKE\tRMS_disp\tRMS_force";

    dump_particles(dump_directory.string(), current_step / dump_period, granular_system->get_x(),
                   granular_system->get_v(), granular_system->get_a(),
                   granular_system->get_omega(), granular_system->get_alpha(), r_part);
    dump_necks(dump_directory.string(), current_step / dump_period, granular_system->get_x(), aggregate_model->get_bonded_contacts(), r_part);

    return true;
}

std::tuple<std::vector<Eigen::Vector3d>, std::vector<Eigen::Vector3d>> RestructuringFixedFractionSimulation::get_neck_information() const {
    auto const & bonded_contacts = aggregate_model->get_bonded_contacts();
    auto const & x = granular_system->get_x();
    size_t neck_count = std::count(bonded_contacts.begin(), bonded_contacts.end(), true) / 2u;

    std::vector<Eigen::Vector3d> neck_positions, neck_orientations;

    neck_positions.reserve(neck_count);
    neck_orientations.reserve(neck_count);

    for (size_t i = 0; i < x.size() - 1; i ++) {
        for (size_t j = i + 1; j < x.size(); j ++) {
            if (!bonded_contacts[i * x.size() + j])
                continue;

            // This is a bonded contact
            Eigen::Vector3d position = (x[j] + x[i]) / 2.0; // Compute the position

            neck_positions.emplace_back(position[0], position[1], position[2]);
        }
    }

    for (size_t i = 0; i < x.size() - 1; i ++) {
        for (size_t j = i + 1; j < x.size(); j ++) {
            if (!bonded_contacts[i * x.size() + j])
                continue;

            // This is a bonded contact
            Eigen::Vector3d orientation = (x[j] - x[i]).normalized(); // Compute the orientation vector

            neck_orientations.emplace_back(orientation[0], orientation[1], orientation[2]);
        }
    }

    return std::make_tuple(neck_positions, neck_orientations);
}

std::tuple<std::string, std::vector<Eigen::Vector3d>, std::vector<Eigen::Vector3d>, std::vector<Eigen::Vector3d>, std::vector<std::vector<Eigen::Vector3d>>> RestructuringFixedFractionSimulation::perform_iterations() {

    std::vector<Eigen::Vector3d> x_before_iter = granular_system->get_x();

    for (int i = 0; i < dump_period; i ++) {
        if (current_step % neighbor_update_period == 0) {
            granular_system->update_neighbor_list();
        }
        granular_system->do_step(dt);
        current_step ++;
    }

    double rms_displacement = 0.0,
            rms_force = 0.0;

    for (int i = 0; i < x_before_iter.size(); i ++) {
        Eigen::Vector3d displacement = x_before_iter[i] - granular_system->get_x()[i];
        rms_displacement += displacement.dot(displacement);

        Eigen::Vector3d force = granular_system->get_a()[i] * mass;
        rms_force += force.dot(force);
    }

    rms_displacement = sqrt(rms_displacement / double(x_before_iter.size()));
    rms_force = sqrt(rms_force / double(x_before_iter.size()));

    std::stringstream message_out;
    auto fmt = format_string(
            "{}\t{:.1e}\t{:.2e}\t{:.2e}\t{:.2e}",   // format string
            current_step / dump_period,  // dump number
            double(current_step) * dt,  // time
            compute_ke(granular_system->get_v(), granular_system->get_omega(), mass, inertia),  // total kinetic energy
            rms_displacement,   // rms displacement of particles from the last dump
            rms_force   // rms force acting on particles
    );
    message_out << fmt;

    dump_particles(dump_directory.string(), current_step / dump_period, granular_system->get_x(),
                   granular_system->get_v(), granular_system->get_a(),
                   granular_system->get_omega(), granular_system->get_alpha(), r_part);
    dump_necks(dump_directory.string(), current_step / dump_period, granular_system->get_x(), aggregate_model->get_bonded_contacts(), r_part);

    auto [neck_positions, neck_orientations] = get_neck_information();

    return {message_out.str(), granular_system->get_x(), neck_positions, neck_orientations, {}};
}
