#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>

#include <QPointer>
#include <QFileDialog>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QVTKOpenGLNativeWidget.h>
#include <QVTKInteractor.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>

#include "restructuring_fixed_fraction.h"
#include "aggregation.h"

#include "config.h"

#define ENABLED_SIMULATIONS RestructuringFixedFractionSimulation, AggregationSimulation

template<typename T>
struct init_combo_box_functor {
    static void apply(MainWindow * main_window) {
        main_window->ui->simulationTypeSelector->addItem(T::combo_label, T::combo_id);
    }
};

template<typename T>
struct changed_combo_box_functor {
    static void apply(MainWindow * main_window) {
        if (T::combo_id == main_window->ui->simulationTypeSelector->currentData().toInt()) {
            main_window->reset_parameter_table();
            main_window->initialize_parameter_table<T>();
        }
    }
};

template<typename T>
struct init_simulation_functor {
    static void apply(MainWindow * main_window) {
        if (T::combo_id == main_window->ui->simulationTypeSelector->currentData().toInt()) {
            main_window->initialize_simulation<T>();
        }
    }
};

template<template<typename> typename functor, typename Head>
void iterate_types(MainWindow * main_window) {
    functor<Head>::apply(main_window);
}

template<template<typename> typename functor, typename Head, typename Mid, typename... Tail>
void iterate_types(MainWindow * main_window) {
    functor<Head>::apply(main_window);
    iterate_types<functor, Mid, Tail...>(main_window);
}

template<typename Head>
parameter_heap_t get_parameters_from_input_current_simulation_type(MainWindow * main_window) {
    return main_window->get_parameters_from_input<Head>();
}

template<typename Head, typename Mid, typename... Tail>
parameter_heap_t get_parameters_from_input_current_simulation_type(MainWindow * main_window) {
    if (main_window->ui->simulationTypeSelector->currentData().toInt() == Head::combo_id)
        return main_window->get_parameters_from_input<Head>();
    return get_parameters_from_input_current_simulation_type<Mid, Tail...>(main_window);
}

template<typename Head>
const char * get_current_simulation_type_config_signature(MainWindow * main_window) {
    return Head::config_file_signature;
}

template<typename Head, typename Mid, typename... Tail>
const char * get_current_simulation_type_config_signature(MainWindow * main_window) {
    if (main_window->ui->simulationTypeSelector->currentData().toInt() == Head::combo_id)
        return Head::config_file_signature;
    return get_current_simulation_type_config_signature<Mid, Tail...>(main_window);
}

//template<typename Head>
//std::shared_ptr<Simulation> simulation_factory(int combo_id [[maybe_unused]],
//                                               std::ostream & output_stream,
//                                               std::vector<Eigen::Vector3d> & x0_buffer,
//                                               parameter_heap_t const & parameter_heap) {
//    return std::make_shared<Head>(output_stream, x0_buffer, parameter_heap);
//}
//
//template<typename Head, typename Mid, typename... Tail>
//std::shared_ptr<Simulation> simulation_factory(int combo_id,
//                                               std::ostream & output_stream,
//                                               std::vector<Eigen::Vector3d> & x0_buffer,
//                                               parameter_heap_t const & parameter_heap) {
//    if (combo_id == Head::combo_id)
//        return std::make_shared<Head>(output_stream, x0_buffer, parameter_heap);
//    return simulation_factory<Mid, Tail...>(combo_id, output_stream, x0_buffer, parameter_heap);
//}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
{
    ui->setupUi(this);

    update_tool_buttons();

    // Set up button actions
    connect(&compute_thread, &ComputeThread::step_done, this, &MainWindow::compute_step_done);
    connect(&compute_thread, &ComputeThread::pause_done, this, &MainWindow::pause_done);
    connect(ui->playButton, &QAbstractButton::clicked, this, &MainWindow::play_button_handler);
    connect(ui->playAllButton, &QAbstractButton::clicked, this, &MainWindow::play_all_button_handler);
    connect(ui->pauseButton, &QAbstractButton::clicked, this, &MainWindow::pause_button_handler);
    connect(ui->actionPause, &QAction::triggered, this, &MainWindow::pause_button_handler);
    connect(ui->resetButton, &QAbstractButton::clicked, this, &MainWindow::reset_button_handler);
    connect(ui->saveButton, &QAbstractButton::clicked, this, &MainWindow::save_button_handler);
    connect(ui->simulationTypeSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(simulation_type_combo_handler()));
    connect(ui->parameterTable, &QTableWidget::itemChanged, this, &MainWindow::parameters_changed);

    // Load the monospaced font and make stdout box use it
    auto mono_font_id = QFontDatabase::addApplicationFont(":/fonts/RobotoMono-VariableFont_wght.ttf");
    QString mono_font_family = QFontDatabase::applicationFontFamilies(mono_font_id).at(0);
    QFont mono_font(mono_font_family);
    ui->stdoutBox->setFont(mono_font);

    // Initialize the simulation type selector
    iterate_types<init_combo_box_functor, ENABLED_SIMULATIONS>(this);

    // Initialize the parameter table
    QStringList horizontal_header;
    horizontal_header << "Parameter" << "Type" << "Value" << "Description";
    ui->parameterTable->setHorizontalHeaderLabels(horizontal_header);

//    initialize_parameter_table<AggregationSimulation>();

    QPointer<QVTKOpenGLNativeWidget> vtkRenderWidget =
            new QVTKOpenGLNativeWidget(ui->previewWidget);

//    vtkRenderWidget->setAttribute(Qt::WA_AcceptTouchEvents, false);
    vtkRenderWidget->setAttribute(Qt::WA_MouseTracking, false);
//    vtkRenderWidget->setAttribute(Qt::WA_TabletTracking, false);
//    vtkRenderWidget->ungrabGesture(Qt::GestureType::PanGesture);
//    vtkRenderWidget->ungrabGesture(Qt::GestureType::TapAndHoldGesture);
//    vtkRenderWidget->ungrabGesture(Qt::GestureType::TapGesture);
//    vtkRenderWidget->ungrabGesture(Qt::GestureType::SwipeGesture);
//    vtkRenderWidget->grabGesture(Qt::GestureType::PinchGesture);
    // This disables interaction completely - last resort
//    vtkRenderWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QPointer<QVBoxLayout> layout = new QVBoxLayout();

    ui->previewWidget->setLayout(layout);

    layout->addWidget(vtkRenderWidget);

    // VTK part
    vtk_named_colors = vtkNew<vtkNamedColors>();
    vtk_render_window = vtkNew<vtkGenericOpenGLRenderWindow>();
    vtkRenderWidget->setRenderWindow(vtk_render_window.Get());
    vtk_renderer = vtkNew<vtkRenderer>();
    vtk_renderer->SetBackground(vtk_named_colors->GetColor3d("White").GetData());
    vtk_render_window->AddRenderer(vtk_renderer);
    vtk_sphere_source = vtkNew<vtkSphereSource>();
    vtk_sphere_source->SetRadius(1.0);
    vtk_sphere_source->SetPhiResolution(30);
    vtk_sphere_source->SetThetaResolution(15);

    vtk_cylinder_source = vtkNew<vtkCylinderSource>();
    vtk_cylinder_source->SetRadius(0.75);
    vtk_cylinder_source->SetHeight(1.5);
    vtk_cylinder_source->SetResolution(30);
}

template <typename SimulationType>
void MainWindow::initialize_parameter_table() {

    watching_parameter_table = false;

    parameter_table_fields.resize(SimulationType::N_PARAMETERS * 4);

    ui->parameterTable->setRowCount(SimulationType::N_PARAMETERS);

    for (size_t i = 0; i < SimulationType::N_PARAMETERS; i ++) {
        auto [id, type, description] = SimulationType::PARAMETERS[i];

        parameter_table_fields[i*4].setText(id);
        parameter_table_fields[i*4].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
        parameter_table_fields[i*4+1].setText(parameter_type_to_string(type));
        parameter_table_fields[i*4+1].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);

        // TODO: remove after debugging
        parameter_table_fields[i*4+2].setText(SimulationType::default_values[i]);

        parameter_table_fields[i*4+3].setText(description);
        parameter_table_fields[i*4+3].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);

        ui->parameterTable->setItem(i, 0, &parameter_table_fields[i*4]);
        ui->parameterTable->setItem(i, 1, &parameter_table_fields[i*4+1]);
        ui->parameterTable->setItem(i, 2, &parameter_table_fields[i*4+2]);
        ui->parameterTable->setItem(i, 3, &parameter_table_fields[i*4+3]);
    }

    ui->parameterTable->resizeColumnToContents(0);
    ui->parameterTable->resizeColumnToContents(1);
    ui->parameterTable->resizeColumnToContents(3);

    watching_parameter_table = true;
    parameters_changed();
}

void MainWindow::reset_parameter_table() {
    watching_parameter_table = false;
    parameter_table_fields.clear();
    watching_parameter_table = true;
}

template<typename SimulationType>
parameter_heap_t MainWindow::get_parameters_from_input() const {
    parameter_heap_t parameter_heap;
    for (int i = 0; i < SimulationType::N_PARAMETERS; i ++) {
        QString value = parameter_table_fields[i*4+2].text();
        auto [id, type, description] = SimulationType::PARAMETERS[i];
        switch (type) {
            case INTEGER:
                parameter_heap[id] = std::make_pair(INTEGER, ParameterValue{.integer_value = value.toInt()});
                break;
            case REAL:
                parameter_heap[id] = std::make_pair(REAL, ParameterValue{.real_value = value.toDouble()});
                break;
            case STRING:
                parameter_heap[id] = std::make_pair(STRING, ParameterValue{.string_value = value.toStdString()});
                break;
            case PATH:
                parameter_heap[id] = std::make_pair(PATH, ParameterValue{.path_value = std::filesystem::path(value.toStdString())});
        }
    }
    return parameter_heap;
}

template<typename SimulationType>
void MainWindow::initialize_simulation() {
    lock_parameters();
    parameter_heap_t parameter_heap = get_parameters_from_input<SimulationType>();

    std::stringstream ss;
    std::vector<Eigen::Vector3d> x0_buffer, neck_positions_buffer, neck_orientations_buffer;
    simulation = std::make_shared<SimulationType>(ss, x0_buffer, neck_positions_buffer, neck_orientations_buffer, parameter_heap);
    ui->stdoutBox->appendPlainText(QString::fromStdString(ss.str()));

    compute_thread.initialize(simulation);

    // TODO: replace constant r_part with parameter
    initialize_preview(x0_buffer, neck_positions_buffer, neck_orientations_buffer, 14e-9);
}

void MainWindow::parameters_changed() {
    if (watching_parameter_table) {
        if (configuration_state == SAVED)
            configuration_state = PATH_CHOSEN;
        update_configuration_state();
    }
}

void MainWindow::new_button_handler() {
    // TODO: prompt if UNSAVED / PATH_CHOSEN; then load defaults for the selected model
}

void MainWindow::open_button_handler() {
    // TODO: prompt if UNSAVED / PATH_CHOSEN; validate parameter file type; load parameters
}

void MainWindow::save_button_handler() {
    configurations_file_path = QFileDialog::getSaveFileName(this,
                     "Save Configuration As", "/", "XML Files (*.xml)");

    // TODO: write parameter file
    parameter_heap_t parameters = get_parameters_from_input_current_simulation_type<ENABLED_SIMULATIONS>(this);
    const char * config_signature = get_current_simulation_type_config_signature<ENABLED_SIMULATIONS>(this);
    write_config_file("", config_signature, parameters);


    configuration_state = SAVED;
    update_configuration_state();
}

void MainWindow::play_button_handler() {
    if (simulation_state == RESET) {
        iterate_types<init_simulation_functor, ENABLED_SIMULATIONS>(this);
    }
    simulation_state = RUN_ONE;
    compute_thread.do_step();
    update_tool_buttons();
}

void MainWindow::play_all_button_handler() {
    if (simulation_state == RESET) {
        iterate_types<init_simulation_functor, ENABLED_SIMULATIONS>(this);
    }
    simulation_state = RUN_CONTINUOUS;
    compute_thread.do_continuous_steps();
    update_tool_buttons();
}

void MainWindow::pause_button_handler() {
    simulation_state = PAUSE_REQUESTED;
    compute_thread.do_pause();
    update_tool_buttons();
}

void MainWindow::reset_button_handler() {
    simulation_state = RESET;
    compute_thread.do_terminate();
    reset_preview();
    ui->stdoutBox->clear();
    update_tool_buttons();
    unlock_parameters();
}

void MainWindow::simulation_type_combo_handler() {
    iterate_types<changed_combo_box_functor, ENABLED_SIMULATIONS>(this);
}

void MainWindow::lock_parameters() {
    watching_parameter_table = false;
    for (size_t i = 0; i < parameter_table_fields.size() / 4; i ++) {
        parameter_table_fields[i*4+2].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    }
    watching_parameter_table = true;
}

void MainWindow::unlock_parameters() {
    watching_parameter_table = false;
    for (size_t i = 0; i < parameter_table_fields.size() / 4; i ++) {
        parameter_table_fields[i*4+2].setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
    watching_parameter_table = true;
}

MainWindow::~MainWindow() = default;

void MainWindow::update_configuration_state() {
    switch (configuration_state) {
        case UNSAVED: {
            ui->saveButton->setEnabled(true);
            setWindowTitle("soot-dem-gui by Egor Demidov - project location not chosen");
            break;
        }
        case PATH_CHOSEN: {
            ui->saveButton->setEnabled(true);
            setWindowTitle("soot-dem-gui by Egor Demidov - " + configurations_file_path + " - changes not saved");
            break;
        }
        case SAVED: {
            ui->saveButton->setEnabled(false);
            setWindowTitle("soot-dem-gui by Egor Demidov - " + configurations_file_path);
        }
    }
}

void MainWindow::update_tool_buttons() {
    switch (simulation_state) {
        case RUN_ONE:
        case PAUSE_REQUESTED: {
            ui->newButton->setEnabled(false);
            ui->saveButton->setEnabled(false);
            ui->openButton->setEnabled(false);
            ui->playButton->setEnabled(false);
            ui->playAllButton->setEnabled(false);
            ui->pauseButton->setEnabled(false);
            ui->resetButton->setEnabled(false);
            ui->simulationTypeSelector->setEnabled(false);
            break;
        }
        case PAUSE: {
            ui->newButton->setEnabled(true);
            ui->saveButton->setEnabled(true);
            ui->openButton->setEnabled(true);
            ui->playButton->setEnabled(true);
            ui->playAllButton->setEnabled(true);
            ui->pauseButton->setEnabled(false);
            ui->resetButton->setEnabled(true);
            ui->simulationTypeSelector->setEnabled(false);
            update_configuration_state();
            break;
        }
        case RUN_CONTINUOUS: {
            ui->newButton->setEnabled(false);
            ui->saveButton->setEnabled(false);
            ui->openButton->setEnabled(false);
            ui->playButton->setEnabled(false);
            ui->playAllButton->setEnabled(false);
            ui->pauseButton->setEnabled(true);
            ui->resetButton->setEnabled(false);
            ui->simulationTypeSelector->setEnabled(false);
            break;
        }
        case RESET: {
            ui->newButton->setEnabled(true);
            ui->saveButton->setEnabled(true);
            ui->openButton->setEnabled(true);
            ui->playButton->setEnabled(true);
            ui->playAllButton->setEnabled(true);
            ui->pauseButton->setEnabled(false);
            ui->resetButton->setEnabled(false);
            ui->simulationTypeSelector->setEnabled(true);
            update_configuration_state();
        }
    }
}


void MainWindow::compute_step_done(QString const & message,
                                   QVector<Eigen::Vector3d> const & x,
                                   QVector<Eigen::Vector3d> const & neck_positions,
                                   QVector<Eigen::Vector3d> const & neck_orientations) {

    ui->stdoutBox->appendPlainText(message);

    if (simulation_state == SimulationState::RUN_ONE) {
        simulation_state = SimulationState::PAUSE;
        update_tool_buttons();
    }
    // TODO: replace hardcoded r_part with value from inputs
    update_preview(std::vector<Eigen::Vector3d>(x.begin(), x.end()),
                   std::vector<Eigen::Vector3d>(neck_positions.begin(), neck_positions.end()),
                   std::vector<Eigen::Vector3d>(neck_orientations.begin(), neck_orientations.end()),
                   14.0e-9);
}

void MainWindow::pause_done() {
    simulation_state = PAUSE;
    update_tool_buttons();
}

void MainWindow::initialize_preview(
        std::vector<Eigen::Vector3d> const & x,
        std::vector<Eigen::Vector3d> const & neck_positions,
        std::vector<Eigen::Vector3d> const & neck_orientations,
        double r_part) {

    // Initialize particle representations
    vtk_particles_representation.reserve(x.size());
    for (size_t i = 0; i < x.size(); i ++) {
        auto mapper = vtkNew<vtkPolyDataMapper>();
        mapper->SetInputConnection(vtk_sphere_source->GetOutputPort());

        auto actor = vtkNew<vtkActor>();
        actor->SetMapper(mapper);

        actor->SetPosition(x[i][0] / r_part, x[i][1] / r_part, x[i][2] / r_part);
        actor->GetProperty()->SetColor(vtk_named_colors->GetColor3d("DimGray").GetData());
        actor->GetProperty()->SetSpecular(0.3);

        vtk_renderer->AddActor(actor);
        vtk_particles_representation.emplace_back(mapper, actor);
    }
    vtk_renderer->ResetCamera();
    vtk_render_window->Render();

    // Initialize neck representations
    vtk_necks_representation.reserve(neck_positions.size());
    for (size_t i = 0; i < neck_positions.size(); i ++) {
        auto mapper = vtkNew<vtkPolyDataMapper>();
        mapper->SetInputConnection(vtk_cylinder_source->GetOutputPort());

        auto actor = vtkNew<vtkActor>();
        actor->SetMapper(mapper);

        actor->SetPosition(neck_positions[i][0] / r_part, neck_positions[i][1] / r_part, neck_positions[i][2] / r_part);

        // Convert orientation vector to rotation
        Eigen::Vector3d initial_axis = Eigen::Vector3d::UnitY();
        Eigen::Vector3d axis = initial_axis.cross(neck_orientations[i]);
        double angle_deg = 180.0 / M_PI * acos(initial_axis.dot(neck_orientations[i]));  // Orientation vector must be normalized

        actor->RotateWXYZ(angle_deg, axis[0], axis[1], axis[2]);
        actor->GetProperty()->SetColor(vtk_named_colors->GetColor3d("Orange").GetData());

        vtk_renderer->AddActor(actor);
        vtk_necks_representation.emplace_back(mapper, actor);
    }
    vtk_renderer->ResetCamera();
    vtk_render_window->Render();
}

// ASSUMING NECK COUNT CAN ONLY DECREASE
void MainWindow::update_preview(std::vector<Eigen::Vector3d> const & x,
                                std::vector<Eigen::Vector3d> const & neck_positions,
                                std::vector<Eigen::Vector3d> const & neck_orientations,
                                double r_part) {

    // Delete necks that have been broken
//    for (long i = vtk_necks_representation.size() - 1; i > neck_positions.size(); i --) {
//        vtk_renderer->RemoveActor(std::get<1>(vtk_necks_representation[i]));
//        vtk_necks_representation.pop_back();
//    }

    for (size_t i = 0; i < x.size(); i ++) {
        vtk_particles_representation[i].second->SetPosition(x[i][0] / r_part, x[i][1] / r_part, x[i][2] / r_part);
    }
    for (size_t i = 0; i < neck_positions.size(); i ++) {
        // Convert orientation vector to rotation
        Eigen::Vector3d initial_axis = Eigen::Vector3d::UnitY();
        Eigen::Vector3d axis = initial_axis.cross(neck_orientations[i]);
        double angle_deg = 180.0 / M_PI * acos(initial_axis.dot(neck_orientations[i]));  // Orientation vector must be normalized

        vtk_necks_representation[i].second->SetOrientation(0.0, 0.0, 0.0);
        vtk_necks_representation[i].second->RotateWXYZ(angle_deg, axis[0], axis[1], axis[2]);
        vtk_necks_representation[i].second->SetPosition(neck_positions[i][0] / r_part, neck_positions[i][1] / r_part, neck_positions[i][2] / r_part);
    }
    vtk_render_window->Render();
}

void MainWindow::reset_preview() {
    for (auto [mapper, actor] : vtk_particles_representation) {
        vtk_renderer->RemoveActor(actor);
    }
    for (auto [mapper, actor] : vtk_necks_representation) {
        vtk_renderer->RemoveActor(actor);
    }
    vtk_necks_representation.clear();
    vtk_particles_representation.clear();
    vtk_render_window->Render();
}

