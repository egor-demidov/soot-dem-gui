#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>

#include <QPointer>

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

constexpr const char * parameter_type_to_string(ParameterType type) {
    switch (type) {
        case INTEGER:
            return "integer";
        case REAL:
            return "real";
        case STRING:
            return "string";
        case PATH:
            return "path";
    }
}

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
    connect(ui->simulationTypeSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(simulation_type_combo_handler()));

    // Load the monospaced font and make stdout box use it
    auto mono_font_id = QFontDatabase::addApplicationFont(":/fonts/RobotoMono-VariableFont_wght.ttf");
    QString mono_font_family = QFontDatabase::applicationFontFamilies(mono_font_id).at(0);
    QFont mono_font(mono_font_family);
    ui->stdoutBox->setFont(mono_font);

    // Initialize the simulation type selector
    iterate_types<init_combo_box_functor,
        RestructuringFixedFractionSimulation,
        AggregationSimulation>(this);

    // Initialize the parameter table
    QStringList horizontal_header;
    horizontal_header << "Parameter" << "Type" << "Value" << "Description";
    ui->parameterTable->setHorizontalHeaderLabels(horizontal_header);

//    initialize_parameter_table<AggregationSimulation>();

    QPointer<QVTKOpenGLNativeWidget> vtkRenderWidget =
            new QVTKOpenGLNativeWidget(ui->previewWidget);

    vtkRenderWidget->setAttribute(Qt::WA_AcceptTouchEvents, false);
    vtkRenderWidget->setAttribute(Qt::WA_NoMousePropagation, true);

    QPointer<QVBoxLayout> layout = new QVBoxLayout();

    ui->previewWidget->setLayout(layout);

    layout->addWidget(vtkRenderWidget);

    // VTK part

    vtk_render_window = vtkNew<vtkGenericOpenGLRenderWindow>();
    vtkRenderWidget->setRenderWindow(vtk_render_window.Get());
    vtk_renderer = vtkNew<vtkRenderer>();
    vtk_render_window->AddRenderer(vtk_renderer);
    vtk_sphere_source = vtkNew<vtkSphereSource>();
    vtk_sphere_source->SetRadius(1.0);
    vtk_sphere_source->SetPhiResolution(30);
    vtk_sphere_source->SetThetaResolution(15);
}

template <typename SimulationType>
void MainWindow::initialize_parameter_table() {
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
}

void MainWindow::reset_parameter_table() {
    parameter_table_fields.clear();
}

template<typename SimulationType>
void MainWindow::initialize_simulation() {
    lock_parameters();
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

    std::stringstream ss;
    std::vector<Eigen::Vector3d> x0_buffer;
    simulation = std::make_shared<SimulationType>(ss, x0_buffer, parameter_heap);
    ui->stdoutBox->appendPlainText(QString::fromStdString(ss.str()));

    compute_thread.initialize(simulation);

    // TODO: replace constant r_part with parameter
    initialize_preview(x0_buffer, 14e-9);
}

void MainWindow::play_button_handler() {
    if (simulation_state == RESET) {
        iterate_types<init_simulation_functor, RestructuringFixedFractionSimulation, AggregationSimulation>(this);
    }
    simulation_state = RUN_ONE;
    compute_thread.do_step();
    update_tool_buttons();
}

void MainWindow::play_all_button_handler() {
    if (simulation_state == RESET) {
        iterate_types<init_simulation_functor, RestructuringFixedFractionSimulation, AggregationSimulation>(this);
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
    iterate_types<changed_combo_box_functor,
     RestructuringFixedFractionSimulation,
     AggregationSimulation>(this);
}

void MainWindow::lock_parameters() {
    for (size_t i = 0; i < parameter_table_fields.size() / 4; i ++) {
        parameter_table_fields[i*4+2].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
    }
}

void MainWindow::unlock_parameters() {
    for (size_t i = 0; i < parameter_table_fields.size() / 4; i ++) {
        parameter_table_fields[i*4+2].setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
}

MainWindow::~MainWindow() = default;

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
        }
    }
}


void MainWindow::compute_step_done(QString const & message,
                       QVector<Eigen::Vector3d> const & x) {

    ui->stdoutBox->appendPlainText(message);

    if (simulation_state == SimulationState::RUN_ONE) {
        simulation_state = SimulationState::PAUSE;
        update_tool_buttons();
    }
    update_preview(std::vector<Eigen::Vector3d>(x.begin(), x.end()), 14.0e-9);
}

void MainWindow::pause_done() {
    simulation_state = PAUSE;
    update_tool_buttons();
}

void MainWindow::initialize_preview(std::vector<Eigen::Vector3d> const & x, double r_part) {
    vtk_particles_representation.reserve(0);
    for (size_t i = 0; i < x.size(); i ++) {
        auto mapper = vtkNew<vtkPolyDataMapper>();
        mapper->SetInputConnection(vtk_sphere_source->GetOutputPort());

        auto actor = vtkNew<vtkActor>();
        actor->SetMapper(mapper);

        actor->SetPosition(x[i][0] / r_part, x[i][1] / r_part, x[i][2] / r_part);

        vtk_renderer->AddActor(actor);
        vtk_particles_representation.emplace_back(mapper, actor);
    }
    vtk_renderer->ResetCamera();
    vtk_render_window->Render();
}

void MainWindow::update_preview(std::vector<Eigen::Vector3d> const & x, double r_part) {
    for (size_t i = 0; i < x.size(); i ++) {
        vtk_particles_representation[i].second->SetPosition(x[i][0] / r_part, x[i][1] / r_part, x[i][2] / r_part);
    }
    vtk_render_window->Render();
}

void MainWindow::reset_preview() {
    for (auto [mapper, actor] : vtk_particles_representation) {
        vtk_renderer->RemoveActor(actor);
    }
    vtk_particles_representation.clear();
    vtk_render_window->Render();
}

