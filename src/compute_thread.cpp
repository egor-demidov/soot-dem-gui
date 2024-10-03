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

#include "compute_thread.h"

ComputeThread::ComputeThread(QObject * parent)
    : QThread(parent) {}

ComputeThread::~ComputeThread() {
    do_terminate();
}

void ComputeThread::initialize(std::shared_ptr<Simulation> simulation_ptr) {
    QMutexLocker locker(&mutex);

    if (worker_state != UNINITIALIZED) {
        std::cerr << "Attempting to re-initialize an initialized worker thread" << std::endl;
        exit(EXIT_FAILURE);
    }
    this->simulation = simulation_ptr;
    worker_state = PAUSE;
    start(HighestPriority);
}

void ComputeThread::do_step() {
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        std::cerr << "Attempting to compute with stopped worker thread" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (worker_state != PAUSE) {
        std::cerr << "Attempting to invoke do_step() on already computing worker thread" << std::endl;
        exit(EXIT_FAILURE);
    }

    worker_state = ADVANCE_ONE;
    condition.wakeOne();
}

void ComputeThread::do_continuous_steps() {
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        std::cerr << "Attempting to compute with stopped worker thread" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (worker_state != PAUSE) {
        std::cerr << "Attempting to invoke do_step() on already computing worker thread" << std::endl;
        exit(EXIT_FAILURE);
    }

    worker_state = ADVANCE_CONTINUOUS;
    condition.wakeOne();
}

void ComputeThread::do_pause() {
    QMutexLocker locker(&mutex);

    if (!isRunning()) {
        std::cerr << "Attempting to pause a stopped worker thread" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (worker_state != ADVANCE_CONTINUOUS) {
        std::cerr << "Attempting a pause a worker thread that is not running in continuous mode" << std::endl;
        exit(EXIT_FAILURE);
    }

    worker_state = PAUSE_REQUEST;
}

void ComputeThread::do_terminate() {
    mutex.lock();
    worker_state = ABORT;
    condition.wakeOne();
    mutex.unlock();

    wait();

    simulation.reset();
    worker_state = UNINITIALIZED;
}

void ComputeThread::run() {
    forever {
        mutex.lock();
        auto current_state = worker_state;
        mutex.unlock();

        if (current_state == ADVANCE_ONE || current_state == ADVANCE_CONTINUOUS) {
            auto [message, x, neck_positions, neck_orientations, polygons] = simulation->perform_iterations();
            QVector<QVector<Eigen::Vector3d>> polygons_qvector;
            polygons_qvector.resize(polygons.size());
            for (long i = 0; i < polygons.size(); i ++) {
                polygons_qvector[i] = QVector<Eigen::Vector3d>(polygons[i].begin(), polygons[i].end());
            }

            emit step_done(QString::fromStdString(message),
                           QVector<Eigen::Vector3d>(x.begin(), x.end()),
                           QVector<Eigen::Vector3d>(neck_positions.begin(), neck_positions.end()),
                           QVector<Eigen::Vector3d>(neck_orientations.begin(), neck_orientations.end()),
                           polygons_qvector);

            if (current_state == ADVANCE_ONE) {
                mutex.lock();
                worker_state = PAUSE;
                mutex.unlock();
            }
        }

        mutex.lock();
        if (worker_state == ABORT) {
            mutex.unlock();
            return;
        }
        if (worker_state != ADVANCE_CONTINUOUS) {
            if (worker_state == PAUSE_REQUEST) {
                emit pause_done();
                worker_state = PAUSE;
            }
            condition.wait(&mutex);
        }
        mutex.unlock();
    }
}
