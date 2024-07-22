//
// Created by Egor Demidov on 7/22/24.
//

#include <iostream>
#include <thread>
#include <chrono>

#include "compute_thread.h"

ComputeThread::ComputeThread(QObject * parent) : QThread(parent) {}

ComputeThread::~ComputeThread() {
    mutex.lock();
    abort = true;
    condition.wakeOne();
    mutex.unlock();

    wait();
}

void ComputeThread::do_step(double step_dt) {
    QMutexLocker locker(&mutex);

    this->dt = step_dt;

    if (!isRunning()) {
        start(LowPriority);
    } else {
        std::cerr << "Attempt to invoke an already running worker thread" << std::endl;
        exit(EXIT_FAILURE);
//        restart = true;
//        condition.wakeOne();
    }
}

[[noreturn]] void ComputeThread::run() {
    forever {
        // Perform iterations...
        for (int i = 0; i < 3; i ++) {
            if (abort)
                break;

            mutex.lock();
            std::cout << "Iteration " << i << ", step size " << dt << ", tid " << std::hash<std::thread::id>{}(std::this_thread::get_id()) << std::endl;
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        emit step_done("DONE");

        mutex.lock();
        if (!restart)
            condition.wait(&mutex);
        restart = false;
        mutex.unlock();
    }
}
