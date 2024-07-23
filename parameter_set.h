//
// Created by egor on 7/22/24.
//

#ifndef GUI_DESIGN_SOOT_DEM_PARAMETER_SET_H
#define GUI_DESIGN_SOOT_DEM_PARAMETER_SET_H

#include <tuple>

enum ParameterType {INTEGER, REAL, STRING, PATH};

static constexpr std::tuple<const char *, ParameterType, const char *> PARAMETERS_RESTRUCTURING_FIXED_FRACTION[] {
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
static constexpr size_t N_PARAMETERS_RESTRUCTURING_FIXED_FRACTION =
        sizeof(PARAMETERS_RESTRUCTURING_FIXED_FRACTION) / sizeof(PARAMETERS_RESTRUCTURING_FIXED_FRACTION[0]);

#endif //GUI_DESIGN_SOOT_DEM_PARAMETER_SET_H
