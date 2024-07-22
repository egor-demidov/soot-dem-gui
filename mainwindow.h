#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <chrono>
#include <thread>

#include <QMainWindow>
#include <QTableWidgetItem>

#include <vtkSmartPointer.h>
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

private:
    void compute_step_done(QString const & message) {
        std::cout << "Message received from worker: " << message.toStdString() << std::endl;
    }

private:
    std::vector<QTableWidgetItem *> parameters;
    std::unique_ptr<Ui::MainWindow> ui;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> vtk_render_window;
    ComputeThread compute_thread;
};

#endif // MAINWINDOW_H
