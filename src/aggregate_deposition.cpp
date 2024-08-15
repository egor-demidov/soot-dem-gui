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
#include <format>

#include "aggregate_deposition.h"

// Centers the loaded aggregate in the xy-plane
void center_in_the_xy_plane(std::vector<Eigen::Vector3d> & x) {
    Eigen::Vector3d center_of_mass = Eigen::Vector3d::Zero();

    for (auto const & pt : x) {
        center_of_mass += pt;
    }

    center_of_mass /= double(x.size());

    for (auto & pt : x) {
        pt[0] -= center_of_mass[0];
        pt[1] -= center_of_mass[1];
    }
}

std::tuple<std::vector<Eigen::Vector3d>, std::vector<Eigen::Vector3d>> AggregateDepositionSimulation::get_neck_information() const {
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

AggregateDepositionSimulation::AggregateDepositionSimulation(
        parameter_heap_t const & parameter_heap,
        std::filesystem::path const & working_directory
) : Simulation(parameter_heap, working_directory) {}

bool
AggregateDepositionSimulation::initialize(std::ostream &output_stream, std::vector<Eigen::Vector3d> &x0_buffer,
                                                 std::vector<Eigen::Vector3d> &neck_positions_buffer,
                                                 std::vector<Eigen::Vector3d> &neck_orientations_buffer) {

    // General parameters
    auto rho = get_real_parameter("rho");
    auto r_verlet = get_real_parameter("r_verlet");
    auto substrate_size = get_real_parameter("substrate_size");
    auto vz0 = get_real_parameter("vz0");
    auto rot_x = get_real_parameter("rot_x");
    auto rot_y = get_real_parameter("rot_y");
    auto rot_z = get_real_parameter("rot_z");

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

    // Parameters for the substrate contact model
    auto k_n_substrate = get_real_parameter("k_n_substrate");
    auto gamma_n_substrate = get_real_parameter("gamma_n_substrate");
    auto k_t_substrate = get_real_parameter("k_t_substrate");
    auto gamma_t_substrate = get_real_parameter("gamma_t_substrate");
    auto mu_t_substrate = get_real_parameter("mu_t_substrate");
    auto phi_t_substrate = get_real_parameter("phi_t_substrate");
    auto k_r_substrate = get_real_parameter("k_r_substrate");
    auto gamma_r_substrate = get_real_parameter("gamma_r_substrate");
    auto mu_r_substrate = get_real_parameter("mu_r_substrate");
    auto phi_r_substrate = get_real_parameter("phi_r_substrate");
    auto k_o_substrate = get_real_parameter("k_o_substrate");
    auto gamma_o_substrate = get_real_parameter("gamma_o_substrate");
    auto mu_o_substrate = get_real_parameter("mu_o_substrate");
    auto phi_o_substrate = get_real_parameter("phi_o_substrate");

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

    // Parameters for the substrate Van der Waals model
    auto A_substrate = get_real_parameter("A_substrate");
    auto h0_substrate = get_real_parameter("h0_substrate");

    auto aggregate_type = get_string_parameter("aggregate_type");
    auto aggregate_path = get_path_parameter("aggregate_path");

    // Substrate vertices
    const std::tuple<Eigen::Vector3d, Eigen::Vector3d, Eigen::Vector3d, Eigen::Vector3d> substrate_vertices {
            {-substrate_size / 2.0, -substrate_size / 2.0, 0.0},
            {substrate_size / 2.0, -substrate_size / 2.0 , 0.0},
            {substrate_size / 2.0 , substrate_size / 2.0, 0.0},
            {-substrate_size / 2.0, substrate_size / 2.0, 0.0}
    };

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

    Eigen::Vector3d center_of_mass = Eigen::Vector3d::Zero();
    for (auto const & x : x0) {
        center_of_mass += x;
    }
    center_of_mass /= double(x0.size());
    Eigen::Matrix3d rot = (Eigen::AngleAxis(rot_x / 180.0 * M_PI, Eigen::Vector3d::UnitX())
                           * Eigen::AngleAxis(rot_y / 180.0 * M_PI, Eigen::Vector3d::UnitY())
                           * Eigen::AngleAxis(rot_z / 180.0 * M_PI, Eigen::Vector3d::UnitZ())).toRotationMatrix();
    std::transform(x0.begin(), x0.end(), x0.begin(), [&center_of_mass, &rot] (auto const & x) -> Eigen::Vector3d {
        return rot * (x - center_of_mass) + center_of_mass;
    });

    double z_min = (*std::min_element(x0.begin(), x0.end(), [](auto const & x1, auto const & x2) -> bool {
        return x1[2] < x2[2];
    }))[2];

    std::transform(x0.begin(), x0.end(), x0.begin(), [this, z_min] (auto const & x) {
        return x + Eigen::Vector3d::UnitZ() * (-z_min + 1.1 * this->r_part);
    });

    center_in_the_xy_plane(x0);

    x0_buffer = x0;

    // Fill the remaining buffers with zeros
    v0.resize(x0.size());
    theta0.resize(x0.size());
    omega0.resize(x0.size());
    std::fill(v0.begin(), v0.end(), Eigen::Vector3d {0, 0, -vz0});
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

    // Create an instance of rectangular substrate model
    substrate_model = std::make_unique<rect_substrate_model_t >(substrate_vertices, x0.size(), k_n_substrate, gamma_n_substrate, k_t_substrate,
                                            gamma_t_substrate, mu_t_substrate, phi_t_substrate, k_r_substrate, gamma_r_substrate, mu_r_substrate,
                                            phi_r_substrate, k_o_substrate, gamma_o_substrate,
                                            mu_o_substrate, phi_o_substrate, A_substrate, h0_substrate, r_part, mass, inertia, dt, Eigen::Vector3d::Zero(), 0.0);


    unary_force_container = std::make_unique<unary_force_container_t>(*substrate_model);

    binary_force_container = std::make_unique<binary_force_container_t>(*aggregate_model);

    granular_system = std::make_unique<granular_system_t>(x0.size(), r_verlet, x0,
                                                          v0, theta0, omega0, 0.0, Eigen::Vector3d::Zero(), 0.0,
                                                          step_handler_instance, *binary_force_container, *unary_force_container);

    // Count the number of necks
    size_t n_necks = std::count(aggregate_model->get_bonded_contacts().begin(),
                                aggregate_model->get_bonded_contacts().end(), true) / 2;

    output_stream << n_necks << " necks inserted in the aggregate" << std::endl;

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

std::tuple<std::string, std::vector<Eigen::Vector3d>, std::vector<Eigen::Vector3d>, std::vector<Eigen::Vector3d>> AggregateDepositionSimulation::perform_iterations() {

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
    auto fmt = std::format(
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

    return {message_out.str(), granular_system->get_x(), neck_positions, neck_orientations};
}