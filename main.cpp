#include "mainwindow.h"

#include <QApplication>
#include <QSurfaceFormat>
#include <QFontDatabase>
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    qRegisterMetaType<QVector<int> >("QVector<Eigen::Vector3d>");
    QApplication a(argc, argv);
    a.setAttribute(Qt::AA_SynthesizeMouseForUnhandledTouchEvents, false);
    MainWindow w;
    w.show();
    return a.exec();
}
