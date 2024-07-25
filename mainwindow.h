#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <chrono>
#include <thread>

#include <QMainWindow>
#include <QTableWidgetItem>

#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
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

//    void closeEvent(QCloseEvent * event) override;

private:
    void compute_step_done(QString const & message,
                           QVector<Eigen::Vector3d> const & x);
    void pause_done();

    void reset_button_handler();
    void play_button_handler();
    void play_all_button_handler();
    void pause_button_handler();

    void lock_parameters();
    void unlock_parameters();

    void update_tool_buttons();

    template<typename SimulationType>
    void initialize_simulation();

    template<typename SimulationType>
    void initialize_parameter_table();
    void reset_parameter_table();

    void initialize_preview(std::vector<Eigen::Vector3d> const & x, double r_part);
    void update_preview(std::vector<Eigen::Vector3d> const & x, double r_part);
    void reset_preview();

    enum SimulationState {
        RESET, RUN_ONE, RUN_CONTINUOUS, PAUSE_REQUESTED, PAUSE
    };

    std::shared_ptr<Simulation> simulation;
    std::vector<QTableWidgetItem> parameter_table_fields;
    std::unique_ptr<Ui::MainWindow> ui;
    ComputeThread compute_thread;
    SimulationState simulation_state = RESET;

    // VTK stuff
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_render_window;
    vtkSmartPointer<vtkRenderer> vtk_renderer;
    vtkSmartPointer<vtkSphereSource> vtk_sphere_source;
    std::vector<std::pair<
        vtkSmartPointer<vtkPolyDataMapper>,
        vtkSmartPointer<vtkActor>
        >> vtk_particles_representation;
};

#endif // MAINWINDOW_H
