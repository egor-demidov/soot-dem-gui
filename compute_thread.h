//
// Created by Egor Demidov on 7/22/24.
//

#ifndef GUI_DESIGN_SOOT_DEM_COMPUTE_THREAD_H
#define GUI_DESIGN_SOOT_DEM_COMPUTE_THREAD_H

#include <numeric>

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QSize>

#include "restructuring_fixed_fraction.h"

struct GranularSystem {
    int passes;
};

class ComputeThread : public QThread {
    Q_OBJECT

public:
    ComputeThread(QObject * parent = nullptr);
    ~ComputeThread();

    void initialize(std::shared_ptr<Simulation> simulation_ptr);

    void do_step();

signals:
    void step_done(QString const & message);

protected:
    [[noreturn]] void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;

    enum {
        UNINITIALIZED, ADVANCE_ONE, ADVANCE_CONTINUOUS, PAUSE, ABORT
    } worker_state = UNINITIALIZED;

    // Parameters received upon "do_step" invocation
    std::shared_ptr<Simulation> simulation;
};

#endif //GUI_DESIGN_SOOT_DEM_COMPUTE_THREAD_H
