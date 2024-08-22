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

#ifndef GUI_DESIGN_SOOT_DEM_GEOMETRY_THREAD_H
#define GUI_DESIGN_SOOT_DEM_GEOMETRY_THREAD_H

#include <numeric>
#include <vector>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QSize>

#include <Eigen/Eigen>


class GeometryThread : public QThread {
    Q_OBJECT

public:
    explicit GeometryThread(QObject * parent = nullptr);
    ~GeometryThread() override;

    void initialize(std::vector<Eigen::Vector3d> const & particle_positions_arg, double r_part_arg);


signals:
    void done(QString const & message);

protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;

    std::vector<Eigen::Vector3d> particle_positions;
    double r_part;
};

#endif //GUI_DESIGN_SOOT_DEM_GEOMETRY_THREAD_H
