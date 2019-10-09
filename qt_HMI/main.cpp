#include "mainwindow.h"

#include "ui_mainwindow.h"


#include <QApplication>
#include <QSysInfo>
int main(int argc, char *argv[])
{

    double scale = 1.0;
    if(QSysInfo::currentCpuArchitecture() == "arm64"){
        scale = 3;
    }

    // Create a temporary QApplication object in its own scope so that it gets deleted
    // afterwards. This is needed for getting the system font size.
//    {
//        QApplication tmp(argc, argv);
//        double ptFont = tmp.font().pointSizeF();
//        if (ptFont < 0.0) {
//            ptFont = tmp.font().pixelSize();
//        }

//        if (ptFont > 11.0) {
//            scale = ptFont / 11.0;
//        }
//    }
    qputenv("QT_SCALE_FACTOR", QString::number(scale).toLocal8Bit());

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
