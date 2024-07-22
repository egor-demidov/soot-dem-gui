#include "mainwindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QFontDatabase>
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
//    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
