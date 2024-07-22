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

struct GranularSystem {
    int passes;
};

class ComputeThread : public QThread {
    Q_OBJECT

public:
    ComputeThread(QObject * parent = nullptr);
    ~ComputeThread();

    void do_step(double dt);

signals:
    void step_done(QString const & message);

protected:
    [[noreturn]] [[noreturn]] void run() override;

private:
    QMutex mutex;
    QWaitCondition condition;
    bool abort = false;
    bool advance = true;
    GranularSystem granular_system;

    // Parameters received upon "do_step" invocation
    double dt = std::numeric_limits<double>::quiet_NaN();
};

#endif //GUI_DESIGN_SOOT_DEM_COMPUTE_THREAD_H
