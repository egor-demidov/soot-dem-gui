//
// Created by Egor on 8/15/2024.
//

#ifndef SOOT_DEM_GUI_ABOUTDIALOG_H
#define SOOT_DEM_GUI_ABOUTDIALOG_H

#include <memory>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class AboutDialog; }
QT_END_NAMESPACE

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

    ~AboutDialog() override = default;

private:
    std::unique_ptr<Ui::AboutDialog> ui;
};

#endif //SOOT_DEM_GUI_ABOUTDIALOG_H
