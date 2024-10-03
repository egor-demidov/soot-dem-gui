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

#ifndef GUI_DESIGN_SOOT_DEM_COMPUTE_THREAD_H
#define GUI_DESIGN_SOOT_DEM_COMPUTE_THREAD_H

#include <numeric>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QSize>

#include "restructuring_fixed_fraction.h"


class ComputeThread : public QThread {
    Q_OBJECT

public:
    explicit ComputeThread(QObject * parent = nullptr);
    ~ComputeThread() override;

    void initialize(std::shared_ptr<Simulation> simulation_ptr);

    void do_step();
    void do_continuous_steps();
    void do_pause();
    void do_terminate();

signals:
    void step_done(QString const & message,
                   QVector<Eigen::Vector3d> const & x,
                   QVector<Eigen::Vector3d> const & neck_positions_buffer,
                   QVector<Eigen::Vector3d> const & neck_orientations_buffer,
                   QVector<QVector<Eigen::Vector3d>> const & polygons);
    void pause_done();

protected:
    void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;

    enum {
        UNINITIALIZED, ADVANCE_ONE, ADVANCE_CONTINUOUS,  PAUSE_REQUEST, PAUSE, ABORT
    } worker_state = UNINITIALIZED;

    // Parameters received upon "do_step" invocation
    std::shared_ptr<Simulation> simulation;
};

#endif //GUI_DESIGN_SOOT_DEM_COMPUTE_THREAD_H
