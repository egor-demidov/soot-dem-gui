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

#include "example_aggregate.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
{
    ui->setupUi(this);

    // Set up button actions
    connect(&compute_thread, &ComputeThread::step_done, this, &MainWindow::compute_step_done);
    connect(ui->playButton, &QAbstractButton::clicked, this, [this]() {
        std::cout << "Calling worker thread" << std::endl;
        this->compute_thread.do_step(14.3);
    });

    // Load the monospaced font and make stdout box use it
    auto mono_font_id = QFontDatabase::addApplicationFont(":/fonts/RobotoMono-VariableFont_wght.ttf");
    QString mono_font_family = QFontDatabase::applicationFontFamilies(mono_font_id).at(0);
    QFont mono_font(mono_font_family);
    ui->stdoutBox->setFont(mono_font);

    // Initialize the parameter table
    QStringList horizontal_header;
    horizontal_header << "Parameter" << "Type" << "Value";
    ui->parameterTable->setHorizontalHeaderLabels(horizontal_header);

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
