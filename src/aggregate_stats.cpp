//
// Created by Egor on 8/21/2024.
//

#include "aggregate_stats.h"

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
