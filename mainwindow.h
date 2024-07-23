#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <chrono>
#include <thread>

#include <QMainWindow>
#include <QTableWidgetItem>

#include <vtkSmartPointer.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include "compute_thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void compute_step_done(QString const & message) {
        std::cout << "Message received from worker: " << message.toStdString() << std::endl;
        simulation_state = SimulationState::PAUSE;
        update_tool_buttons();
    }

    void reset_button_handler();
    void play_button_handler();
    void play_all_button_handler();
    void stop_button_handler();


    void update_tool_buttons();

    enum SimulationState {
        RESET, RUN_ONE, RUN_CONTINUOUS, PAUSE
    };

    std::vector<QTableWidgetItem> parameter_table_fields;
    std::unique_ptr<Ui::MainWindow> ui;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_render_window;
    ComputeThread compute_thread;
    SimulationState simulation_state = RESET;
};

#endif // MAINWINDOW_H
