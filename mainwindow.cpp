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

        simulation = std::make_shared<RestructuringFixedFractionSimulation>(std::cout, parameter_heap);

        compute_thread.initialize(simulation);
    }
    simulation_state = RUN_ONE;
    update_tool_buttons();
    compute_thread.do_step();
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
