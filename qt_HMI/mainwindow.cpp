#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget_MAIN->setCurrentIndex(0);
    ui->leSTATE->setStyleSheet(styleSheetERROR);

    socket = new QUdpSocket(this);


    main_timer = new QTimer(this);
    connect(main_timer, SIGNAL(timeout()), this, SLOT(timerCallback()));


    bool ok = socket->bind(QHostAddress::AnyIPv4, 55555);
    connect(socket, &QUdpSocket::readyRead,
            this, &MainWindow::readPendingDatagrams);
    if(ok){
        ui->leSTATE->setStyleSheet(styleSheetWARN);
        ui->leSTATE->setText("Waiting for ESP");
    } else {
        ui->leSTATE->setText("FAILED");
    }
    setAppEnable(false);

    //    connect(ui->spbMotorVthr, &QDoubleSpinBox::valueChanged,
    //            this, &MainWindow::start_editing_settings);
}

MainWindow::~MainWindow()
{
    this->socket->close();
    delete ui;
}


void MainWindow::readPendingDatagrams()
{
    this->last_udp_status = QDateTime::currentDateTime();
    this->setAppEnable(true);
    main_timer->start(100);


    QByteArray datagram;
    while (socket->hasPendingDatagrams()) {
        datagram.resize(int(socket->pendingDatagramSize()));
        socket->readDatagram(datagram.data(), datagram.size());
    }
    std::string in = datagram.toStdString();
    QString data = QString(datagram);
    //    qDebug() << data;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data.toUtf8());
    json_object_received_state = jsonResponse.object();
    //    qDebug() << json_object_received_state["ssid"];
    last_udp_status =  QDateTime::currentDateTime();

    ui->leSTATE->setText("OK");
    ui->leSTATE->setStyleSheet(styleSheetOK);

    parseJson();
    update_UI_from_json_status();
}

void MainWindow::parseJson()
{
    this->updated_state.sensor = json_object_received_state["sensor"].toInt();
    this->updated_state.voltage = json_object_received_state["voltage"].toDouble();
    this->updated_state.motor_running = json_object_received_state["motor_running"].toBool();

    if (json_object_received_state.contains("ssid") && json_object_received_state["ssid"].isString())
        this->updated_state.ssid_string = json_object_received_state["ssid"].toString();

    this->received_settings.sensor_threshold_to_turn_on = json_object_received_state["sensor_threshold_to_turn_on"].toInt();
    this->received_settings.sensor_threshold_to_turn_off = json_object_received_state["sensor_threshold_to_turn_off"].toInt();

    if (json_object_received_state.contains("motor_running_voltage_threshold") && json_object_received_state["motor_running_voltage_threshold"].isDouble())
        this->received_settings.motor_running_voltage_threshold = json_object_received_state["motor_running_voltage_threshold"].toDouble();


}


void MainWindow::update_UI_from_json_status()
{
    if(this->updated_state.motor_running == true){
        ui->leMOTOR->setStyleSheet(styleSheetOK);
        ui->leMOTOR->setText("RUNNING");
    } else {
        ui->leMOTOR->setStyleSheet(styleSheetWARN);
        ui->leMOTOR->setText("STOPPED");
    }
    ui->lcdVoltage->display(this->updated_state.voltage);
    ui->pbSensor->setValue(100 *this->updated_state.sensor / 1024);
    ui->pbSensor_2->setValue(100 *this->updated_state.sensor / 1024);

    if(not isChangingSettings){
        ui->slTHR2ON->setValue(100 *this->received_settings.sensor_threshold_to_turn_on / 1024);
        ui->slTHR2OFF->setValue(100 *this->received_settings.sensor_threshold_to_turn_off / 1024);
        ui->spbMotorVthr->setValue(this->received_settings.motor_running_voltage_threshold);
    }
}

void MainWindow::timerCallback()
{
    QDateTime now = QDateTime::currentDateTime();
    //    qDebug() << "NOW:\t" << now << "\t\t" << now.toMSecsSinceEpoch();
    //    qDebug() << "PREV:\t" << last_udp_status << "\t\t" << last_udp_status.toMSecsSinceEpoch();

    if (last_udp_status.msecsTo(now) > this->millis_timer_timeout) {
        main_timer->stop();
        setAppEnable(false);
        ui->leSTATE->setText("TIMEOUT");
        ui->leSTATE->setStyleSheet(styleSheetERROR);
    }

}

void MainWindow::setAppEnable(bool enabled)
{
    ui->leMOTOR->setEnabled(enabled);
    ui->leLightsMode->setEnabled(enabled);
    ui->cbGLOBALENABLE->setEnabled(enabled);
    ui->pbSensor->setEnabled(enabled);
    ui->lcdVoltage->setEnabled(enabled);


    ui->pbSensor_2->setEnabled(enabled);
    ui->slTHR2ON->setEnabled(enabled);
    ui->slTHR2OFF->setEnabled(enabled);
    ui->spbMotorVthr->setEnabled(enabled);
    ui->cbWinterMode->setEnabled(enabled);

}


void MainWindow::on_spbMotorVthr_valueChanged(double value)
{
    this->isChangingSettings = true;
    qDebug() << "MotorVthr: " << value;
}

void MainWindow::on_spbMotorVthr_editingFinished()
{
    this->isChangingSettings = false;
    // TODO: send the updated value to ESP
}

void MainWindow::on_slTHR2ON_sliderPressed()
{
    //    QInputDialog::getDouble(this->centralWidget(), QString("Custom"), QString("Enter value"));
}

void MainWindow::on_slTHR2ON_valueChanged(int value)
{
    qDebug() << "slTHR2ON_value: " << value ;
    // TODO: send the updated value to ESP
}

void MainWindow::on_slTHR2OFF_valueChanged(int value)
{
    qDebug() << "slTHR2OFF_value: " << value ;
    // TODO: send the updated value to ESP
}
