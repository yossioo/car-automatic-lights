#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

/// Convert a 4 byte array to 32-bit integer
void bytearraytouint32(const char in[4], uint32_t *out)
{
    uint8_t a = in[0];
    uint8_t b = in[1];
    uint8_t c = in[2];
    uint8_t d = in[3];
    qDebug() << a;
    *out = ((a << 24) | (b << 16) | (c << 8) | d);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QUdpSocket(this);

    socket->bind(QHostAddress::AnyIPv4, 55555);
    connect(socket, &QUdpSocket::readyRead,
               this, &MainWindow::readPendingDatagrams);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readPendingDatagrams()
{
    QByteArray datagram;
    while (socket->hasPendingDatagrams()) {
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size());
    }
    QByteArray volt_data;
    for(int i = 0; i < 4; i++) {
        volt_data.append(datagram.at(i));
    }
    QByteArray brightness_data;
    for(int i = 0; i < 4; i++) {
        brightness_data.append(datagram.at(i+4));
    }
    unsigned int val = -1;
    bytearraytouint32(volt_data.data(), &val);
//    qDebug() << "Bytes: " << datagram.toHex();
//    qDebug() << "Decoded value: " << val;
    ui->lcdVoltage->display(0.1 * int(val));


    bytearraytouint32(brightness_data.data(), &val);
    int b_val = int(100.0/1024*int(val));
    ui->lcdBrightness->display(b_val);
    ui->pbBrigthness->setValue(b_val);
//    ui->lcdBrightness->display(QString(int(100.0/1024*int(val))) + "%");
}
