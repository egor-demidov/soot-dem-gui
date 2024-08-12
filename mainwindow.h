#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <chrono>
#include <thread>

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QSettings>

#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNamedColors.h>

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

    template<typename SimulationType>
    bool initialize_parameter_table();
    template<typename SimulationType>
    bool initialize_parameter_table_with_data(parameter_heap_t const & parameters);
    void reset_parameter_table();

    template<typename SimulationType>
    bool initialize_simulation();

    template<typename SimulationType>
    parameter_heap_t get_parameters_from_input() const;

    std::unique_ptr<Ui::MainWindow> ui;

private slots:
    void compute_step_done(QString const & message,
                           QVector<Eigen::Vector3d> const & x,
                           QVector<Eigen::Vector3d> const & neck_positions,
                           QVector<Eigen::Vector3d> const & neck_orientations);

    void pause_done();
    void reset_button_handler();
    void play_button_handler();
    void play_all_button_handler();
    void pause_button_handler();
    void new_button_handler();
    void open_button_handler();
    bool save_button_handler();
    bool save_as_button_handler();

    void parameters_changed();

    void simulation_type_combo_handler();

private:
    bool save();
    bool save_as();

    void lock_parameters();

    void unlock_parameters();

    void update_tool_buttons();
    void update_configuration_state();

    void initialize_preview(
            std::vector<Eigen::Vector3d> const & x,
            std::vector<Eigen::Vector3d> const & neck_positions,
            std::vector<Eigen::Vector3d> const & neck_orientations,
            double r_part);
    void update_preview(std::vector<Eigen::Vector3d> const & x,
                        std::vector<Eigen::Vector3d> const & neck_positions,
                        std::vector<Eigen::Vector3d> const & neck_orientations,
                        double r_part);
    void reset_preview();

    enum SimulationState {
        RESET, RUN_ONE, RUN_CONTINUOUS, PAUSE_REQUESTED, PAUSE
    };

    enum ConfigurationFileState {
        NONE, UNSAVED, PATH_CHOSEN, SAVED
    };

    bool watching_parameter_table = false;

    std::shared_ptr<Simulation> simulation;
    std::vector<QTableWidgetItem> parameter_table_fields;
    ComputeThread compute_thread;
    SimulationState simulation_state = RESET;
    ConfigurationFileState configuration_state = NONE;
    QString configurations_file_path;
//    QSettings settings;

    // VTK stuff
    vtkSmartPointer<vtkNamedColors> vtk_named_colors;  // Available colors: https://htmlpreview.github.io/?https://github.com/Kitware/vtk-examples/blob/gh-pages/VTKNamedColorPatches.html
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_render_window;
    vtkSmartPointer<vtkRenderer> vtk_renderer;
    vtkSmartPointer<vtkSphereSource> vtk_sphere_source;
    vtkSmartPointer<vtkCylinderSource> vtk_cylinder_source;
    std::vector<std::pair<
        vtkSmartPointer<vtkPolyDataMapper>,
        vtkSmartPointer<vtkActor>
        >> vtk_particles_representation;
    std::vector<std::pair<
            vtkSmartPointer<vtkPolyDataMapper>,
            vtkSmartPointer<vtkActor>
         >> vtk_necks_representation;
};

#endif // MAINWINDOW_H
