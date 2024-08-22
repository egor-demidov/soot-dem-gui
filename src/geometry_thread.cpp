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

#include <iostream>
#include <thread>
#include <chrono>

#include "geometry_thread.h"

#include "aggregate_stats.h"

GeometryThread::GeometryThread(QObject * parent)
    : QThread(parent) {}

GeometryThread::~GeometryThread() {
    this->terminate();
}

void GeometryThread::initialize(std::vector<Eigen::Vector3d> const & particle_positions_arg, double r_part_arg) {
    QMutexLocker locker(&mutex);
    this->particle_positions = particle_positions_arg;
    this->r_part = r_part_arg;
    start(HighestPriority);
}

void GeometryThread::run() {
    std::stringstream ss;

    std::vector<AggregateGraph> aggregates = find_aggregates(this->particle_positions, this->r_part, this->r_part / 10.0);

    ss << "Found " << aggregates.size() << " aggregates\n\n";

    for (int i = 0; i < aggregates.size(); i ++) {
        auto [edgeIndices, nodeIndices] = aggregates[i];

        std::vector<Eigen::Vector3d> sub_aggregate;
        sub_aggregate.reserve(nodeIndices.size());
        for (auto j : nodeIndices) {
            sub_aggregate.emplace_back(this->particle_positions[j]);
        }

        double rg = r_gyration(sub_aggregate);
        double coord = coordination_number(sub_aggregate, this->r_part, this->r_part / 10.0);

        ss << "Aggregate " << i + 1 << "\n";
        ss << "Size: " << nodeIndices.size() << "\n";
        ss << "Radius of gyration: " << rg << "\n";
        ss << "Coordination number: " << coord << "\n";

        ss << "\n";
    }

    ss << "End of geometry analysis\n";

    emit done(QString::fromStdString(ss.str()));
}
