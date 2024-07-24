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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , parameter_table_fields(RestructuringFixedFractionSimulation::N_PARAMETERS * 4)
{
    ui->setupUi(this);

    update_tool_buttons();

    // Set up button actions
    connect(&compute_thread, &ComputeThread::step_done, this, &MainWindow::compute_step_done);
    connect(ui->playButton, &QAbstractButton::clicked, this, &MainWindow::play_button_handler);
    connect(ui->playAllButton, &QAbstractButton::clicked, this, &MainWindow::play_all_button_handler);
    connect(ui->pauseButton, &QAbstractButton::clicked, this, &MainWindow::pause_button_handler);
    connect(ui->resetButton, &QAbstractButton::clicked, this, &MainWindow::reset_button_handler);

    // Load the monospaced font and make stdout box use it
    auto mono_font_id = QFontDatabase::addApplicationFont(":/fonts/RobotoMono-VariableFont_wght.ttf");
    QString mono_font_family = QFontDatabase::applicationFontFamilies(mono_font_id).at(0);
    QFont mono_font(mono_font_family);
    ui->stdoutBox->setFont(mono_font);

    // Initialize the parameter table
    QStringList horizontal_header;
    horizontal_header << "Parameter" << "Type" << "Value" << "Description";
    ui->parameterTable->setHorizontalHeaderLabels(horizontal_header);

    ui->parameterTable->setRowCount(RestructuringFixedFractionSimulation::N_PARAMETERS);

    for (size_t i = 0; i < RestructuringFixedFractionSimulation::N_PARAMETERS; i ++) {
        auto [id, type, description] = RestructuringFixedFractionSimulation::PARAMETERS[i];

        parameter_table_fields[i*4].setText(id);
        parameter_table_fields[i*4].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
        parameter_table_fields[i*4+1].setText(parameter_type_to_string(type));
        parameter_table_fields[i*4+1].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);

        // TODO: remove after debugging
        parameter_table_fields[i*4+2].setText(RestructuringFixedFractionSimulation::default_values[i]);

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

    QPointer<QVTKOpenGLNativeWidget> vtkRenderWidget =
            new QVTKOpenGLNativeWidget(ui->previewWidget);

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

void MainWindow::play_button_handler() {
    if (simulation_state == RESET) {
        parameter_heap_t parameter_heap;
        for (int i = 0; i < RestructuringFixedFractionSimulation::N_PARAMETERS; i ++) {
            QString value = parameter_table_fields[i*4+2].text();
            auto [id, type, description] = RestructuringFixedFractionSimulation::PARAMETERS[i];
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
        simulation = std::make_shared<RestructuringFixedFractionSimulation>(ss, x0_buffer, parameter_heap);
        ui->stdoutBox->appendPlainText(QString::fromStdString(ss.str()));

        compute_thread.initialize(simulation);

        // TODO: replace constant r_part with parameter
        initialize_preview(x0_buffer, 14e-9);
    }
    simulation_state = RUN_ONE;
    update_tool_buttons();
    compute_thread.do_step();
}

void MainWindow::play_all_button_handler() {
    if (simulation_state == RESET) {
        parameter_heap_t parameter_heap;
        for (int i = 0; i < RestructuringFixedFractionSimulation::N_PARAMETERS; i ++) {
            QString value = parameter_table_fields[i*4+2].text();
            auto [id, type, description] = RestructuringFixedFractionSimulation::PARAMETERS[i];
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
        simulation = std::make_shared<RestructuringFixedFractionSimulation>(ss, x0_buffer, parameter_heap);
        ui->stdoutBox->appendPlainText(QString::fromStdString(ss.str()));

        compute_thread.initialize(simulation);

        initialize_preview(x0_buffer, 14e-9);
    }
    simulation_state = RUN_CONTINUOUS;
    update_tool_buttons();
    compute_thread.do_continuous_steps();
}

void MainWindow::pause_button_handler() {
    simulation_state = PAUSE;
    update_tool_buttons();
    compute_thread.do_pause();
}

void MainWindow::reset_button_handler() {
    simulation_state = RESET;
    update_tool_buttons();
    compute_thread.do_terminate();
    reset_preview();
    ui->stdoutBox->clear();
}

MainWindow::~MainWindow() = default;

void MainWindow::update_tool_buttons() {
    switch (simulation_state) {
        case RUN_ONE: {
            ui->newButton->setEnabled(false);
            ui->saveButton->setEnabled(false);
            ui->openButton->setEnabled(false);
            ui->playButton->setEnabled(false);
            ui->playAllButton->setEnabled(false);
            ui->pauseButton->setEnabled(false);
            ui->resetButton->setEnabled(false);
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

