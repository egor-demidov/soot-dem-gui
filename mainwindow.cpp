#include <cmath>
#include <iostream>

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

    //    connect(ui->pushButton, &QAbstractButton::clicked, this, &MainWindow::on_pushBtn_clicked);

    ui->stdoutBox->appendPlainText("Loaded an aggregate of size 442");
    ui->stdoutBox->appendPlainText("Dump: 0 \tKE: 0 \tP: 0 \tL: 0 \tR_g: 3.4247e-07");
    ui->stdoutBox->appendPlainText("Dump: 1 \tKE: 2.46189e-16 \tP: 3.90375e-32 \tL: 1.44021e-38 \tR_g: 3.42415e-07");

    auto id = QFontDatabase::addApplicationFont(":/fonts/RobotoMono-VariableFont_wght.ttf");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    QFont monospace(family);
    ui->stdoutBox->setFont(monospace);

    QStringList horizontal_header;
    horizontal_header << "Parameter" << "Type" << "Value";
    ui->parameterTable->setHorizontalHeaderLabels(horizontal_header);

    parameters.emplace_back(new QTableWidgetItem("k_n"));
    parameters.emplace_back(new QTableWidgetItem("double"));
    parameters.emplace_back(new QTableWidgetItem(""));
    parameters.emplace_back(new QTableWidgetItem("gamma_n"));
    parameters.emplace_back(new QTableWidgetItem("double"));
    parameters.emplace_back(new QTableWidgetItem(""));

    for (int i = 0; i < parameters.size(); i ++) {
        ui->parameterTable->setItem(i / 3, i % 3, parameters[i]);
    }

    parameters[1]->setFlags(Qt::NoItemFlags | Qt::ItemIsEnabled);

    ui->menubar->setNativeMenuBar(true);
    ui->menubar->addMenu("Menu")->addAction("Abc");

//    auto layout = new QVBoxLayout();

    QPointer<QVTKOpenGLNativeWidget> vtkRenderWidget =
            new QVTKOpenGLNativeWidget(ui->previewWidget);

    auto layout = new QVBoxLayout();

    ui->previewWidget->setLayout(layout);

    layout->addWidget(vtkRenderWidget);

//    vtkRenderWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // VTK part

    vtkNew<vtkGenericOpenGLRenderWindow> vtk_window;
    vtkRenderWidget->setRenderWindow(vtk_window.Get());

    vtkNew<vtkSphereSource> sphere;
    sphere->SetRadius(1.0);
    sphere->SetThetaResolution(100);
    sphere->SetPhiResolution(100);

    vtkNew<vtkRenderer> renderer;

    for (int i = 0; i < EXAMPLE_AGGREGATE_SIZE / 3; i ++) {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(sphere->GetOutputPort());

        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);

        actor->SetPosition(
                EXAMPLE_AGGREGATE[i*3],
                EXAMPLE_AGGREGATE[i*3+1],
                EXAMPLE_AGGREGATE[i*3+2]
        );

        renderer->AddActor(actor);
    }

    vtk_window->AddRenderer(renderer);
}

MainWindow::~MainWindow() = default;
