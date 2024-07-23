//
// Created by egor on 7/23/24.
//

#include "restructuring_fixed_fraction.h"

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

RestructuringFixedFractionSimulation::RestructuringFixedFractionSimulation(
            std::ostream & output_stream,
            std::vector<Eigen::Vector3d> & x0_buffer,
            parameter_heap_t const & parameter_heap
    )
    : Simulation(parameter_heap) {

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
        x0 = load_vtk_aggregate(aggregate_path, r_part);
    } else if (aggregate_type == "mackowski") {
        x0 = load_mackowski_aggregate(aggregate_path, r_part);
    } else if (aggregate_type == "flage") {
        x0 = load_flage_aggregate(aggregate_path, r_part);
    } else {
        std::cerr << "Unrecognized aggregate type: " << aggregate_type << std::endl;
        exit(EXIT_FAILURE);
    }

    if (x0.size() == 0) {
        std::cerr << "Loaded an empty aggregate" << std::endl;
        exit(EXIT_FAILURE);
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
}

std::tuple<std::string, std::vector<Eigen::Vector3d>> RestructuringFixedFractionSimulation::perform_iterations() {
    for (int i = 0; i < dump_period; i ++) {
        if (current_step % neighbor_update_period == 0) {
            granular_system->update_neighbor_list();
        }
        granular_system->do_step(dt);
        current_step ++;
    }

    std::stringstream message_out;
    message_out << "Dump #" << current_step / dump_period << " \tt: " << double(current_step) * dt;

    return {message_out.str(), granular_system->get_x()};
}