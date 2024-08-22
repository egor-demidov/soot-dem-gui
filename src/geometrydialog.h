//
// Created by Egor on 8/15/2024.
//

#ifndef SOOT_DEM_GUI_GEOMETRYDIALOG_H
#define SOOT_DEM_GUI_GEOMETRYDIALOG_H

#include <memory>
#include <vector>

#include <QDialog>

#include <Eigen/Eigen>

#include "geometry_thread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class GeometryDialog; }
QT_END_NAMESPACE

class GeometryDialog : public QDialog {
    Q_OBJECT

public:
    explicit GeometryDialog(
            std::vector<Eigen::Vector3d> const & particles,
            double r_part,
            QWidget *parent = nullptr
    );
    ~GeometryDialog() override;

    void ok_button_handler();

private slots:
    void geometry_thread_done(QString const & message);

private:
    std::unique_ptr<Ui::GeometryDialog> ui;
    GeometryThread geometry_thread;
};

#endif //SOOT_DEM_GUI_GEOMETRYDIALOG_H
