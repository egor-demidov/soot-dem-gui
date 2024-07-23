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

#include "parameter_set.h"

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
    , parameter_table_fields(N_PARAMETERS_RESTRUCTURING_FIXED_FRACTION * 4)
{
    ui->setupUi(this);

    update_tool_buttons();

    // Set up button actions
    connect(&compute_thread, &ComputeThread::step_done, this, &MainWindow::compute_step_done);
    connect(ui->playButton, &QAbstractButton::clicked, this, [this]() {
        std::cout << "Calling worker thread" << std::endl;
        this->simulation_state = SimulationState::RUN_ONE;
        this->compute_thread.do_step(14.3);
        this->update_tool_buttons();
    });

    // Load the monospaced font and make stdout box use it
    auto mono_font_id = QFontDatabase::addApplicationFont(":/fonts/RobotoMono-VariableFont_wght.ttf");
    QString mono_font_family = QFontDatabase::applicationFontFamilies(mono_font_id).at(0);
    QFont mono_font(mono_font_family);
    ui->stdoutBox->setFont(mono_font);

    // Initialize the parameter table
    QStringList horizontal_header;
    horizontal_header << "Parameter" << "Type" << "Value" << "Description";
    ui->parameterTable->setHorizontalHeaderLabels(horizontal_header);

    ui->parameterTable->setRowCount(N_PARAMETERS_RESTRUCTURING_FIXED_FRACTION);

    for (size_t i = 0; i < N_PARAMETERS_RESTRUCTURING_FIXED_FRACTION; i ++) {
        auto [id, type, description] = PARAMETERS_RESTRUCTURING_FIXED_FRACTION[i];

        parameter_table_fields[i*4].setText(id);
        parameter_table_fields[i*4].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
        parameter_table_fields[i*4+1].setText(parameter_type_to_string(type));
        parameter_table_fields[i*4+1].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
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
