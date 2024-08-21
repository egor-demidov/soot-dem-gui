// Copyright (C) 2024 Egor Demidov
// This file is part of soot-dem-gui
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>
#include <climits>

#include <QPointer>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QVTKOpenGLNativeWidget.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkPolygon.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>

#include "aboutdialog.h"

#include "restructuring_fixed_fraction.h"
#include "aggregation.h"
#include "aggregate_deposition.h"

#include "config.h"

#define ENABLED_SIMULATIONS RestructuringFixedFractionSimulation, AggregationSimulation//, AggregateDepositionSimulation

template<typename T1, typename T2>
inline void set_enabled(T1 * obj1, T2 * obj2, bool state) {
    obj1->setEnabled(state);
    obj2->setEnabled(state);
}

template<typename T>
inline void set_enabled(T * obj, bool state) {
    obj->setEnabled(state);
}

template<typename T>
struct init_combo_box_functor {
    static bool apply(MainWindow * main_window) {
        main_window->ui->simulationTypeSelector->addItem(T::combo_label, T::combo_id);
        return true;
    }
};
template<typename T>
struct show_about_simulation_dialog_functor {
    static bool apply(MainWindow * main_window) {
        if (T::combo_id == main_window->ui->simulationTypeSelector->currentData().toInt()) {
            QMessageBox::about(main_window, "About " + QString::fromStdString(T::combo_label) + " simulation",
                               T::DESCRIPTION);
        }
        return true;
    }
};


template<typename T>
struct changed_combo_box_functor {
    static bool apply(MainWindow * main_window) {
        if (T::combo_id == main_window->ui->simulationTypeSelector->currentData().toInt()) {
            main_window->reset_parameter_table();
            return main_window->initialize_parameter_table<T>();
        }
        return true;
    }
};

template<typename T>
struct init_parameter_table_from_data_functor {
    static bool apply(MainWindow * main_window, parameter_heap_t const & parameters) {
        if (T::combo_id == main_window->ui->simulationTypeSelector->currentData().toInt()) {
            main_window->reset_parameter_table();
            return main_window->initialize_parameter_table_with_data<T>(parameters);
        }
        return true;
    }
};

template<typename T>
struct init_simulation_functor {
    static bool apply(MainWindow * main_window) {
        if (T::combo_id == main_window->ui->simulationTypeSelector->currentData().toInt()) {
            return main_window->initialize_simulation<T>();
        }
        return true;
    }
};

template<template<typename> typename functor, typename Head>
bool iterate_types(MainWindow * main_window) {
    return functor<Head>::apply(main_window);
}

template<template<typename> typename functor, typename Head, typename Mid, typename... Tail>
bool iterate_types(MainWindow * main_window) {
    if (!functor<Head>::apply(main_window))
        return false;
    return iterate_types<functor, Mid, Tail...>(main_window);
}

template<template<typename> typename functor, typename Head>
bool iterate_types_w_heap(MainWindow * main_window, parameter_heap_t const & parameters) {
    return functor<Head>::apply(main_window, parameters);
}

template<template<typename> typename functor, typename Head, typename Mid, typename... Tail>
bool iterate_types_w_heap(MainWindow * main_window, parameter_heap_t const & parameters) {
    if (!functor<Head>::apply(main_window, parameters))
        return false;
    return iterate_types_w_heap<functor, Mid, Tail...>(main_window, parameters);
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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
{
    ui->setupUi(this);

    update_tool_buttons();

    /* Set up compute thread signal handlers */

    connect(&compute_thread, &ComputeThread::step_done, this, &MainWindow::compute_step_done);
    connect(&compute_thread, &ComputeThread::pause_done, this, &MainWindow::pause_done);

    /* Set up button actions */

    // Play button
    connect(ui->playButton, &QAbstractButton::clicked, this, &MainWindow::play_button_handler);
    connect(ui->actionAdvance_One_Step, &QAction::triggered, this, &MainWindow::play_button_handler);
    // Play all button
    connect(ui->playAllButton, &QAbstractButton::clicked, this, &MainWindow::play_all_button_handler);
    connect(ui->actionAdvance_Continuously, &QAction::triggered, this, &MainWindow::play_all_button_handler);
    // Pause button
    connect(ui->pauseButton, &QAbstractButton::clicked, this, &MainWindow::pause_button_handler);
    connect(ui->actionPause, &QAction::triggered, this, &MainWindow::pause_button_handler);
    // Reset button
    connect(ui->resetButton, &QAbstractButton::clicked, this, &MainWindow::reset_button_handler);
    connect(ui->actionStop, &QAction::triggered, this, &MainWindow::reset_button_handler);
    // Save button
    connect(ui->saveButton, &QAbstractButton::clicked, this, &MainWindow::save_button_handler);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::save_button_handler);
    // New button
    connect(ui->newButton, &QAbstractButton::clicked, this, &MainWindow::new_button_handler);
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::new_button_handler);
    // Open button
    connect(ui->openButton, &QAbstractButton::clicked, this, &MainWindow::open_button_handler);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::open_button_handler);
    // Save as button
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::save_as_button_handler);
    // About button
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about_dialog_handler);
    // About simulation button
    connect(ui->aboutSimulationButton, &QAbstractButton::clicked, this, &MainWindow::about_simulation_handler);
    connect(ui->actionInfoAbout_simulation_type, &QAction::triggered, this, &MainWindow::about_simulation_handler);

    connect(ui->simulationTypeSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(simulation_type_combo_handler()));
    connect(ui->parameterTable, &QTableWidget::itemChanged, this, &MainWindow::parameters_changed);

    ui->stdoutBox->setMaximumBlockCount(500);

    // TODO: connect itemChanged to input validator

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

    QPointer<QVTKOpenGLNativeWidget> vtkRenderWidget =
            new QVTKOpenGLNativeWidget(ui->previewWidget);

    vtkRenderWidget->setAttribute(Qt::WA_MouseTracking, false);  // Required for macOS
    vtkRenderWidget->setAttribute(Qt::WA_NoMousePropagation, true);  // Required for linux KDE

    QPointer<QVBoxLayout> layout = new QVBoxLayout();

    ui->previewWidget->setLayout(layout);

    layout->addWidget(vtkRenderWidget);

    ui->splitter_2->setSizes({INT_MAX, INT_MAX});

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

//    vtkNew<vtkPoints> points;
//    points->InsertNextPoint(0.0, 0.0, 0.0);
//    points->InsertNextPoint(1.0, 0.0, 0.0);
//    points->InsertNextPoint(1.0, 1.0, 0.0);
//    points->InsertNextPoint(0.0, 1.0, 0.0);
//
//    vtkNew<vtkPolygon> polygon;
//    polygon->GetPointIds()->SetNumberOfIds(4); // make a quad
//    polygon->GetPointIds()->SetId(0, 0);
//    polygon->GetPointIds()->SetId(1, 1);
//    polygon->GetPointIds()->SetId(2, 2);
//    polygon->GetPointIds()->SetId(3, 3);
//
//    // Add the polygon to a list of polygons
//    vtkNew<vtkCellArray> polygons;
//    polygons->InsertNextCell(polygon);
//
//    // Create a PolyData
//    vtkNew<vtkPolyData> polygonPolyData;
//    polygonPolyData->SetPoints(points);
//    polygonPolyData->SetPolys(polygons);
//
//    // Create a mapper and actor
//    vtkNew<vtkPolyDataMapper> mapper;
//    mapper->SetInputData(polygonPolyData);
//
//    vtkNew<vtkActor> actor;
//    actor->SetMapper(mapper);
//
//    vtk_renderer->AddActor(actor);
}

template<typename SimulationType>
bool MainWindow::initialize_parameter_table_with_data(parameter_heap_t const & parameters) {

    watching_parameter_table = false;

    parameter_table_fields.resize(SimulationType::N_PARAMETERS * 4);

    ui->parameterTable->setRowCount(SimulationType::N_PARAMETERS);

    for (size_t i = 0; i < SimulationType::N_PARAMETERS; i ++) {
        auto [id, type, description] = SimulationType::PARAMETERS[i];

        parameter_table_fields[i*4].setText(id);
        parameter_table_fields[i*4].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);
        parameter_table_fields[i*4+1].setText(parameter_type_to_string(type));
        parameter_table_fields[i*4+1].setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);

        std::stringstream ss;
        auto [type_heap, value_heap] = parameters.at(id);
        if (type_heap != type) {
            throw UiException("Parameter type mismatch");
        }

        switch (type) {
            case REAL: {
                ss << value_heap.real_value;
                parameter_table_fields[i*4+2].setText(QString::fromStdString(ss.str()));
                break;
            }
            case INTEGER: {
                ss << value_heap.integer_value;
                parameter_table_fields[i*4+2].setText(QString::fromStdString(ss.str()));
                break;
            }
            case STRING: {
                parameter_table_fields[i*4+2].setText(QString::fromStdString(value_heap.string_value));
                break;
            }
            case PATH: {
                parameter_table_fields[i*4+2].setText(QString::fromStdString(value_heap.path_value.string()));
            }
        }

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
    configuration_state = SAVED;
    update_configuration_state();

    return true;
}

template <typename SimulationType>
bool MainWindow::initialize_parameter_table() {

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
    configuration_state = NONE;
    update_configuration_state();

    return true;
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
bool MainWindow::initialize_simulation() {
    lock_parameters();
    parameter_heap_t parameter_heap = get_parameters_from_input<SimulationType>();

    std::stringstream ss;
    std::vector<Eigen::Vector3d> x0_buffer, neck_positions_buffer, neck_orientations_buffer;

    simulation = std::make_shared<SimulationType>(parameter_heap, std::filesystem::path(configurations_file_path.toStdString()).parent_path());

    if (!simulation->initialize(ss,
                                x0_buffer,
                                neck_positions_buffer,
                                neck_orientations_buffer)) {

        unlock_parameters();
        return false;
    }

    ui->stdoutBox->appendPlainText(QString::fromStdString(ss.str()));

    compute_thread.initialize(simulation);

    // TODO: replace constant r_part with parameter
    initialize_preview(x0_buffer, neck_positions_buffer, neck_orientations_buffer, 14e-9);

    return true;
}

void MainWindow::parameters_changed() {
    if (watching_parameter_table) {
        if (configuration_state == NONE)
            configuration_state = UNSAVED;
        if (configuration_state == SAVED)
            configuration_state = PATH_CHOSEN;
        update_configuration_state();
    }
}

void MainWindow::new_button_handler() {
    if (configuration_state == UNSAVED || configuration_state == PATH_CHOSEN) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Unsaved changes", "There are unsaved changes. "
                                                               "Would you like to save the current configuration starting a blank simulation?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel)
            return;
        if (reply == QMessageBox::Yes)
            save_as_button_handler();
    }

    iterate_types<changed_combo_box_functor, ENABLED_SIMULATIONS>(this);
}

void MainWindow::open_button_handler() {
    if (configuration_state == UNSAVED || configuration_state == PATH_CHOSEN) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Unsaved changes", "There are unsaved changes. "
                                        "Would you like to save the current configuration before loading from file?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel)
            return;
        if (reply == QMessageBox::Yes)
            save_as_button_handler();
    }

    QString new_configurations_file_path = QFileDialog::getOpenFileName(
            this,
            "Open configuration",
            configurations_file_path.isEmpty()
                ? QDir::homePath()
                : QString::fromStdString(std::filesystem::path(configurations_file_path.toStdString()).parent_path().string()),
            "XML Files (*.xml)"
    );

    // Check if user canceled
    if (new_configurations_file_path.isEmpty())
        return;

    // TODO: attempt to load simulation type and parameters from file, if successful - set configurations_file_path=new_configurations_file_path, set config state to SAVED

    try {
        auto [simulation_type, parameter_heap] = load_config_file(new_configurations_file_path.toStdString());
        unsigned int combo_id = config_signature_to_id<ENABLED_SIMULATIONS>(simulation_type.c_str());
        ui->simulationTypeSelector->blockSignals(true);
        auto combo_index = ui->simulationTypeSelector->findData(combo_id);
        ui->simulationTypeSelector->setCurrentIndex(combo_index);
        ui->simulationTypeSelector->blockSignals(false);
        iterate_types_w_heap<init_parameter_table_from_data_functor, ENABLED_SIMULATIONS>(this, parameter_heap);
        configurations_file_path = new_configurations_file_path;
        update_configuration_state();
    } catch (UiException const & e) {
        std::cerr << e.what() << std::endl;
        QMessageBox::warning(this, "File open error", "Unable to load and parse file at `" + new_configurations_file_path + "`");
    }
}

bool MainWindow::save_as() {
    configurations_file_path = QFileDialog::getSaveFileName(
            this,
            "Save configuration as",
            configurations_file_path.isEmpty()
                ? QDir::homePath()
                : QString::fromStdString(std::filesystem::path(configurations_file_path.toStdString()).parent_path().string()),
            "XML Files (*.xml)"
    );

    // Check if user canceled
    if (configurations_file_path.isEmpty())
        return false;

    return save();
}

bool MainWindow::save() {
    parameter_heap_t parameters = get_parameters_from_input_current_simulation_type<ENABLED_SIMULATIONS>(this);
    const char * config_signature = get_current_simulation_type_config_signature<ENABLED_SIMULATIONS>(this);

    try {
        write_config_file(configurations_file_path.toStdString(), config_signature, parameters);
    } catch (UiException const & e) {
        QMessageBox::warning(this, "File save error", "Unable to save config file to `" + configurations_file_path + "`");
        return false;
    }

    return true;
}

void MainWindow::about_simulation_handler() {
    iterate_types<show_about_simulation_dialog_functor, ENABLED_SIMULATIONS>(this);
}

void MainWindow::about_dialog_handler() {
    AboutDialog aboutDialog(this);
    aboutDialog.show();
    aboutDialog.exec();
    /*QMessageBox::about(this, "About soot-dem-gui",
                                        "A GUI for soot-dem project:\nhttps://github.com/egor-demidov/soot-dem\n\n"
                                        "Contact model description available at:\nhttps://doi.org/10.48550/arXiv.2407.14254\n\n"
                                        "Source code available at:\nhttps://github.com/egor-demidov/soot-dem-gui\n\n"
                                        "Feedback and questions to:\nmail@edemidov.com\n\n"
                                        "Project funded by:\nU.S. N.S.F. Award #AGS-2222104\n\n"
                                       "Copyright (c) 2024, Egor Demidov\n\n"
                                       "GNU GPL License V3");*/
}

bool MainWindow::save_as_button_handler() {
    bool result = save_as();
    if (result)
        configuration_state = SAVED;
    update_configuration_state();
    return result;
}

bool MainWindow::save_button_handler() {
    bool result;
    if (configuration_state == UNSAVED || configuration_state == NONE)
        result = save_as();
    else
        result = save();

    if (result)
        configuration_state = SAVED;
    update_configuration_state();

    return result;
}

void MainWindow::play_button_handler() {
    if (simulation_state == RESET) {
        if (configuration_state != SAVED) {
            // The config must be saved before simulation starts
            bool result = save_button_handler();
            if (!result) return;
        }
        bool result = iterate_types<init_simulation_functor, ENABLED_SIMULATIONS>(this);
        if (!result) {
            QMessageBox::warning(this, "Initialization error", "Unable to initialize the simulation. Check parameters and retry.");
            return;
        }
    }
    simulation_state = RUN_ONE;
    compute_thread.do_step();
    update_tool_buttons();
}

void MainWindow::play_all_button_handler() {
    if (simulation_state == RESET) {
        if (configuration_state != SAVED) {
            // The config must be saved before simulation starts
            bool result = save_button_handler();
            if (!result) return;
        }
        bool result = iterate_types<init_simulation_functor, ENABLED_SIMULATIONS>(this);
        if (!result) {
            QMessageBox::warning(this, "Initialization error", "Unable to initialize the simulation. Check parameters and retry.");
            return;
        }
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
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm reset", "Are you sure you want to reset the simulation? "
                                                           "It will be impossible to resume the simulation after the reset",
                                  QMessageBox::Reset | QMessageBox::Cancel);

    if (reply == QMessageBox::Cancel)
        return;

    simulation_state = RESET;
    compute_thread.do_terminate();
    reset_preview();
    ui->stdoutBox->clear();
    update_tool_buttons();
    unlock_parameters();
}

void MainWindow::simulation_type_combo_handler() {
    new_button_handler();
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
        case NONE: {
            // Paired buttons
            set_enabled(ui->saveButton, ui->actionSave, true);
            set_enabled(ui->newButton, ui->actionNew, false);

            // Single buttons
            set_enabled(ui->actionSaveAs, true);

            setWindowTitle("soot-dem-gui by Egor Demidov");
            break;
        }
        case UNSAVED: {
            // Paired buttons
            set_enabled(ui->saveButton, ui->actionSave, true);
            set_enabled(ui->newButton, ui->actionNew, true);

            // Single buttons
            set_enabled(ui->actionSaveAs, true);

            setWindowTitle("soot-dem-gui by Egor Demidov - project location not chosen");
            break;
        }
        case PATH_CHOSEN: {
            // Paired buttons
            set_enabled(ui->saveButton, ui->actionSave, true);
            set_enabled(ui->newButton, ui->actionNew, true);

            // Single buttons
            set_enabled(ui->actionSaveAs, true);

            setWindowTitle("soot-dem-gui by Egor Demidov - " + configurations_file_path + " - changes not saved");
            break;
        }
        case SAVED: {
            // Paired buttons
            set_enabled(ui->saveButton, ui->actionSave, false);
            set_enabled(ui->newButton, ui->actionNew, true);

            // Single buttons
            set_enabled(ui->actionSaveAs, true);

            setWindowTitle("soot-dem-gui by Egor Demidov - " + configurations_file_path);
        }
    }
}

void MainWindow::update_tool_buttons() {
    switch (simulation_state) {
        case RUN_ONE:
        case PAUSE_REQUESTED: {
            // Paired buttons
            set_enabled(ui->newButton, ui->actionNew, false);
            set_enabled(ui->saveButton, ui->actionSave, false);
            set_enabled(ui->openButton, ui->actionOpen, false);
            set_enabled(ui->playButton, ui->actionAdvance_One_Step, false);
            set_enabled(ui->playAllButton, ui->actionAdvance_Continuously, false);
            set_enabled(ui->pauseButton, ui->actionPause, false);
            set_enabled(ui->resetButton, ui->actionStop, false);

            // Single buttons
            set_enabled(ui->actionSaveAs, false);
            set_enabled(ui->simulationTypeSelector, false);

            break;
        }
        case PAUSE: {
            // Paired buttons
            set_enabled(ui->newButton, ui->actionNew, false);
            set_enabled(ui->saveButton, ui->actionSave, false);
            set_enabled(ui->openButton, ui->actionOpen, false);
            set_enabled(ui->playButton, ui->actionAdvance_One_Step, true);
            set_enabled(ui->playAllButton, ui->actionAdvance_Continuously, true);
            set_enabled(ui->pauseButton, ui->actionPause, false);
            set_enabled(ui->resetButton, ui->actionStop, true);

            // Single buttons
            set_enabled(ui->actionSaveAs, false);
            set_enabled(ui->simulationTypeSelector, false);

            break;
        }
        case RUN_CONTINUOUS: {
            // Paired buttons
            set_enabled(ui->newButton, ui->actionNew, false);
            set_enabled(ui->saveButton, ui->actionSave, false);
            set_enabled(ui->openButton, ui->actionOpen, false);
            set_enabled(ui->playButton, ui->actionAdvance_One_Step, false);
            set_enabled(ui->playAllButton, ui->actionAdvance_Continuously, false);
            set_enabled(ui->pauseButton, ui->actionPause, true);
            set_enabled(ui->resetButton, ui->actionStop, false);

            // Single buttons
            set_enabled(ui->actionSaveAs, false);
            set_enabled(ui->simulationTypeSelector, false);

            break;
        }
        case RESET: {
            // Paired buttons
            set_enabled(ui->newButton, ui->actionNew, true);
            set_enabled(ui->saveButton, ui->actionSave, true);
            set_enabled(ui->openButton, ui->actionOpen, true);
            set_enabled(ui->playButton, ui->actionAdvance_One_Step, true);
            set_enabled(ui->playAllButton, ui->actionAdvance_Continuously, true);
            set_enabled(ui->pauseButton, ui->actionPause, false);
            set_enabled(ui->resetButton, ui->actionStop, false);

            // Single buttons
            set_enabled(ui->actionSaveAs, true);
            set_enabled(ui->simulationTypeSelector, true);

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
    for (auto const & [mapper, actor] : vtk_particles_representation) {
        vtk_renderer->RemoveActor(actor);
    }
    for (auto const & [mapper, actor] : vtk_necks_representation) {
        vtk_renderer->RemoveActor(actor);
    }
    vtk_necks_representation.clear();
    vtk_particles_representation.clear();
    vtk_render_window->Render();
}

