//
// Created by egor on 7/23/24.
//

#include "aggregation.h"

bool particle_overlaps(Eigen::Vector3d const & particle,
                       std::vector<Eigen::Vector3d> const & xs,
                       double r_part) {
    return std::any_of(xs.begin(), xs.end(), [&particle, r_part] (Eigen::Vector3d const & x) {
        return (x - particle).norm() < 2.0 * r_part;
    });
}

Eigen::Vector3d get_random_unit_vector() {
    Eigen::Vector3d vec;
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    do {
        vec = {
                dist(get_random_engine()),
                dist(get_random_engine()),
                dist(get_random_engine())
        };
    } while (vec.norm() == 0);
    return vec.normalized();
}

const std::array<Eigen::Vector3d, 6> box_faces {
        -Eigen::Vector3d::UnitX(),
        Eigen::Vector3d::UnitX(),
        -Eigen::Vector3d::UnitY(),
        Eigen::Vector3d::UnitY(),
        -Eigen::Vector3d::UnitZ(),
        Eigen::Vector3d::UnitZ(),
};

void bounce_off_walls(std::vector<Eigen::Vector3d> const & particles,
                      std::vector<Eigen::Vector3d> & velocities,
                      double r_part, double box_size) {
    for (size_t n = 0; n < box_faces.size(); n ++) {
        for (size_t i = 0; i < particles.size(); i ++) {
            if (velocities[i].dot(box_faces[n]) > 0.0 && box_size / 2.0 - particles[i].dot(box_faces[n]) < r_part) {
                velocities[i] -= 2.0 * velocities[i].dot(box_faces[n]) * box_faces[n];
//                std::cout << "Condition met\n";
            }
        }
    }
}

AggregationSimulation::AggregationSimulation(
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

    // Parameters for the Van der Waals model
    auto A = get_real_parameter("A");
    auto h0 = get_real_parameter("h0");

    // Aggregation set up parameters
    const long n_part = get_integer_parameter("n_part");
    const double v0_part = get_real_parameter("v0_part");
    const double d_crit = get_real_parameter("d_crit"); // Critical separation (required tp build graphs)

    // Initialization of member variables
    dt = get_real_parameter("dt");
    dump_period = get_integer_parameter("dump_period");
    neighbor_update_period = get_integer_parameter("neighbor_update_period");
    r_part = get_real_parameter("r_part");
    mass = 4.0 / 3.0 * M_PI * pow(r_part, 3.0) * rho;
    inertia = 2.0 / 5.0 * mass * pow(r_part, 2.0);
    box_size = get_real_parameter("box_size");

    // Declare the initial condition buffers
    std::vector<Eigen::Vector3d> x0, v0, theta0, omega0;

    seed_random_engine(rng_seed);

    x0.resize(n_part);
    std::uniform_real_distribution<double> x0_dist(-(box_size / 2.0 - r_part), box_size / 2.0 - r_part);
    for (long n = 0; n < n_part; n ++) {
        Eigen::Vector3d particle;
        do {
            particle = {
                    x0_dist(get_random_engine()),
                    x0_dist(get_random_engine()),
                    x0_dist(get_random_engine())
            };
        } while (particle_overlaps(particle, x0, r_part));
        x0[n] = particle;
    }

    // Generate initial velocities
    v0.resize(x0.size());
    std::transform(v0.begin(), v0.end(), v0.begin(), [v0_part](auto const & v [[maybe_unused]]) {
        return get_random_unit_vector() * v0_part;
    });

    x0_buffer = x0;

    // Fill the remaining buffers with zeros
    theta0.resize(x0.size());
    omega0.resize(x0.size());
    std::fill(theta0.begin(), theta0.end(), Eigen::Vector3d::Zero());
    std::fill(omega0.begin(), omega0.end(), Eigen::Vector3d::Zero());

    contact_model = std::make_unique<contact_force_model_t >(
            x0.size(),
            k_n, gamma_n,
            k_t, gamma_t, mu_t, phi_t,
            k_r, gamma_r, mu_r, phi_r,
            k_o, gamma_o, mu_o, phi_o,
            r_part, mass, inertia, dt, Eigen::Vector3d::Zero(), 0.0);

    hamaker_model = std::make_unique<hamaker_force_model_t>(
            A, h0, r_part, mass, Eigen::Vector3d::Zero(), 0.0
    );

    binary_force_container = std::make_unique<binary_force_container_t >(*contact_model, *hamaker_model);

    unary_force_container = std::make_unique<unary_force_container_t>();

    granular_system = std::make_unique<granular_system_neighbor_list_mutable_velocity>(x0.size(), r_verlet, x0,
                                                          v0, theta0, omega0, 0.0, Eigen::Vector3d::Zero(), 0.0,
                                                          step_handler_instance, *binary_force_container, *unary_force_container);
}

std::tuple<std::string, std::vector<Eigen::Vector3d>> AggregationSimulation::perform_iterations() {
    for (int i = 0; i < dump_period; i ++) {
        if (current_step % neighbor_update_period == 0) {
            granular_system->update_neighbor_list();
        }
        granular_system->do_step(dt);
        bounce_off_walls(granular_system->get_x(), granular_system->get_v(), r_part, box_size);
        current_step ++;
    }

    std::stringstream message_out;
    message_out << "Dump #" << current_step / dump_period << " \tt: " << double(current_step) * dt;

    return {message_out.str(), granular_system->get_x()};
}
