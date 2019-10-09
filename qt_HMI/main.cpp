#include "mainwindow.h"

#include <QApplication>
#include <QSysInfo>
int main(int argc, char *argv[])
{
    if(QSysInfo::currentCpuArchitecture() == "arm64"){
    qputenv("QT_SCALE_FACTOR", QString::number(3).toLocal8Bit());
    }
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
