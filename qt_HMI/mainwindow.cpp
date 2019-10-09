#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->leSTATUS->setStyleSheet("QLineEdit { background: rgb(255, 0, 0); selection-background-color: rgb(233, 99, 0); }");

    socket = new QUdpSocket(this);


    bool ok = socket->bind(QHostAddress::AnyIPv4, 55555);
    connect(socket, &QUdpSocket::readyRead,
            this, &MainWindow::readPendingDatagrams);
    if(ok){
        ui->leSTATUS->setStyleSheet("QLineEdit { background: rgb(255, 200, 0); selection-background-color: rgb(233, 99, 0); }");
        ui->leSTATUS->setText("Waiting for ESP-CAL");
    } else {
        ui->leSTATUS->setText("Cannot open the port.");
    }
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
    std::string in = datagram.toStdString();
    QString data = QString(datagram);
    //    qDebug() << data;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    json_object_received_state = jsonResponse.object();
    //    qDebug() << json_object_received_state["ssid"];
    last_udp_status =  QDateTime::currentDateTime();
    ui->leSTATUS->setText("Receiving heartbeat");
    ui->leSTATUS->setStyleSheet("QLineEdit { background: rgb(0, 255, 0); selection-background-color: rgb(50, 255, 0); }");
    parseJson();
    update_UI_from_json_status();
}

void MainWindow::parseJson()
{
    this->updated_state.sensor = json_object_received_state["sensor"].toInt();
    this->updated_state.voltage = json_object_received_state["voltage"].toInt();
    this->updated_state.motor_running = json_object_received_state["motor_running"].toBool();
    this->received_settings.sensor_threshold_to_turn_on = json_object_received_state["sensor_threshold_to_turn_on"].toInt();
    this->received_settings.sensor_threshold_to_turn_off = json_object_received_state["sensor_threshold_to_turn_off"].toInt();

    if (json_object_received_state.contains("ssid") && json_object_received_state["ssid"].isString())
        this->updated_state.ssid_string = json_object_received_state["ssid"].toString();

}

void MainWindow::update_UI_from_json_status()
{
    if(this->updated_state.motor_running == true){
        ui->leMOTOR->setStyleSheet("QLineEdit { background: rgb(0, 255, 0); selection-background-color: rgb(50, 255, 0); }");
        ui->leMOTOR->setText("RUNNING");
    } else {
        ui->leMOTOR->setStyleSheet("QLineEdit { background: rgb(255, 128, 0); selection-background-color: rgb(50, 255, 0); }");
        ui->leMOTOR->setText("STOPPED");
    }
    ui->lcdVoltage->display(0.1*this->updated_state.voltage);
    ui->pbSensor->setValue(100 *this->updated_state.sensor / 1024);
    ui->slTHR2ON->setValue(100 *this->received_settings.sensor_threshold_to_turn_on / 1024);
    ui->slTHR2OFF->setValue(100 *this->received_settings.sensor_threshold_to_turn_off / 1024);

}

