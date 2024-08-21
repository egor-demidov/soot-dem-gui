//
// Created by Egor on 8/21/2024.
//

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/convex_hull_3.h>
#include <CGAL/Polygon_mesh_processing/measure.h>

#include "aggregate_stats.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Polyhedron_3<K> Polyhedron_3;
typedef K::Point_3 Point_3;
typedef CGAL::Surface_mesh<Point_3> Surface_mesh;

double compute_convexity(std::vector<Eigen::Vector3d> const & x, double r_part) {
    std::vector<Point_3> points(x.size());

    for (int n = 0; n < x.size(); n ++) {
        points[n] = {
                x[n][0],
                x[n][1],
                x[n][2]
        };
    }

    Polyhedron_3 poly;
    CGAL::convex_hull_3(points.begin(), points.end(), poly);

    double volume_convex_hull = CGAL::Polygon_mesh_processing::volume(poly);
    double volume_aggregate = double(x.size()) * 4.0f / 3.0f * M_PI * pow(r_part, 3.0);

    return volume_aggregate / volume_convex_hull;
}

void recursive_edge_traversal(AggregateGraph & graph,
                              std::vector<bool> & visited_edges,
                              std::vector<GraphEdge> const & edges) {

    auto const & prevEdge = edges[graph.edgeIndices.back()];

    // Iterate over unvisited edges and see if any of them are adjacent
    for (int k = 0; k < edges.size(); k ++) {
        if (visited_edges[k])
            continue;

        // Check if adjacent
        if (edges[k].first == prevEdge.first || edges[k].second == prevEdge.first
            || edges[k].first == prevEdge.second || edges[k].second == prevEdge.second) {

            // Mark as visited
            visited_edges[k] = true;

            graph.edgeIndices.emplace_back(k);

            recursive_edge_traversal(graph, visited_edges, edges);
        }
    }
}

void populate_node_indices(AggregateGraph & graph, std::vector<GraphEdge> const & edges) {
    for (auto edgeIndex : graph.edgeIndices) {
        auto const & edge = edges[edgeIndex];
        if (!std::any_of(graph.nodeIndices.begin(), graph.nodeIndices.end(), [&edge] (auto nodeIndex) {
            return nodeIndex == edge.first;
        }))
            graph.nodeIndices.emplace_back(edge.first);
        if (!std::any_of(graph.nodeIndices.begin(), graph.nodeIndices.end(), [&edge] (auto nodeIndex) {
            return nodeIndex == edge.second;
        }))
            graph.nodeIndices.emplace_back(edge.second);
    }
}

std::vector<AggregateGraph> find_aggregates(std::vector<Eigen::Vector3d> const & x, double r_part, double d_crit) {
    // Build graphs of aggregates to write them out separately

    std::vector<AggregateGraph> graphs;
    std::vector<GraphEdge> edges;

    for (int i = 0; i < x.size() - 1; i ++) {
        // Iterate over neighbors
        for (int j = i+1; j < x.size(); j ++) {
            Eigen::Vector3d distance = x[j] - x[i];
            if (sqrt(distance.dot(distance)) - 2.0 * r_part > d_crit)
                continue;

            // Create an edge
            edges.emplace_back(i, j);
        }
    }

    std::vector<bool> visited_edges(edges.size(), false);

    // Iterate over edges
    for (int k = 0; k < edges.size(); k ++) {
        if (visited_edges[k])
            continue;

        // Mark as visited
        visited_edges[k] = true;

        AggregateGraph graph;
        graph.edgeIndices.emplace_back(k);
        recursive_edge_traversal(graph, visited_edges, edges);
        graphs.emplace_back(graph);
    }

    for (auto & graph : graphs) {
        populate_node_indices(graph, edges);
    }

    return graphs;
}

double coordination_number(std::vector<Eigen::Vector3d> const & x, double r_part, double d_crit) {
    double neighbors = 0.0;
    for (size_t i = 0; i < x.size(); i ++) {
        for (size_t j = 0; j < x.size(); j ++) {
            if (i == j)
                continue;

            if ((x[i] - x[j]).norm() - 2.0 * r_part <= d_crit) neighbors += 1.0;
        }
    }

    return neighbors / double(x.size());
}
