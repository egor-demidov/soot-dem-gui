#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
//    void on_pushBtn_clicked();

private:
    std::vector<QTableWidgetItem *> parameters;
    std::unique_ptr<Ui::MainWindow> ui;
};
#endif // MAINWINDOW_H
