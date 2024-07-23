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


class ComputeThread : public QThread {
    Q_OBJECT

public:
    explicit ComputeThread(QObject * parent = nullptr);
    ~ComputeThread() override;

    void initialize(std::shared_ptr<Simulation> simulation_ptr);

    void do_step();

signals:
    void step_done(QString const & message,
                   QVector<Eigen::Vector3d> const & x);

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
