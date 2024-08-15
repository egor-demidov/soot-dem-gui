//
// Created by Egor on 8/15/2024.
//

#include "aboutdialog.h"
#include "./ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui{std::make_unique<Ui::AboutDialog>()}
{
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AboutDialog::ok_button_handler);

    ui->versionLabel->setText(ui->versionLabel->text() + PROJECT_VERSION_STRING);
}

AboutDialog::~AboutDialog() = default;

void AboutDialog::ok_button_handler() {
    this->close();
}
