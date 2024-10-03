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

#include "mainwindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QFontDatabase>
#include <QStyleHints>
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
    qRegisterMetaType<QVector<int> >("QVector<Eigen::Vector3d>");
    qRegisterMetaType<QVector<int> >("QVector<QVector<Eigen::Vector3d>>");
    QIcon::setThemeName("light");
    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);
    MainWindow w;
    w.show();
    return a.exec();
}
