//
// Created by Egor Demidov on 7/22/24.
//

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

    worker_state = PAUSE;
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
            auto [message, x] = simulation->perform_iterations();
            emit step_done(QString::fromStdString(message), QVector<Eigen::Vector3d>(x.begin(), x.end()));

            if (current_state == ADVANCE_ONE) {
                mutex.lock();
                worker_state = PAUSE;
                mutex.unlock();
            }
        }

        mutex.lock();
        if (worker_state == ABORT) {
            return;
        }
        if (worker_state != ADVANCE_CONTINUOUS) {
            condition.wait(&mutex);
        }
        mutex.unlock();
    }
}
