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
}

