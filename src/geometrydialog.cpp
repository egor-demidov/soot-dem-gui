//
// Created by Egor on 8/15/2024.
//

#include "geometrydialog.h"
#include "./ui_geometrydialog.h"

GeometryDialog::GeometryDialog(
        std::vector<Eigen::Vector3d> const & particles,
        double r_part,
        QWidget *parent
    )
    : QDialog(parent)
    , ui{std::make_unique<Ui::GeometryDialog>()}
{
    ui->setupUi(this);

    connect(&geometry_thread, &GeometryThread::done, this, &GeometryDialog::geometry_thread_done);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &GeometryDialog::ok_button_handler);

    ui->geometryAnalysisOutput->setPlainText("Starting geometry analysis of " + QString::number(particles.size()) + " particles...\n");

    geometry_thread.initialize(particles, r_part);
}

GeometryDialog::~GeometryDialog() = default;

void GeometryDialog::geometry_thread_done(QString const & message) {
    ui->geometryAnalysisOutput->appendPlainText(message);
}

void GeometryDialog::ok_button_handler() {
    this->close();
}
