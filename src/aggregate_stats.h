//
// Created by Egor on 8/21/2024.
//

#ifndef SOOT_DEM_GUI_AGGREGATE_STATS_H
#define SOOT_DEM_GUI_AGGREGATE_STATS_H

#include <vector>
#include <Eigen/Eigen>

#include <aggregate_stats.h>

struct AggregateGraph {
    std::vector<int> edgeIndices;
    std::vector<int> nodeIndices;
};

using GraphEdge = std::pair<int, int>;

std::vector<AggregateGraph> find_aggregates(std::vector<Eigen::Vector3d> const & x, double r_part, double d_crit);

double coordination_number(std::vector<Eigen::Vector3d> const & x, double r_part, double d_crit);

#endif //SOOT_DEM_GUI_AGGREGATE_STATS_H
