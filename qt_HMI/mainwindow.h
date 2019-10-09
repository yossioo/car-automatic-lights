#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



typedef enum {
  LIGHTS_OFF_MOTOR = 0,
  LIGHTS_ON = 1,
  LIGHTS_DELAY = 2,
  LIGHTS_OFF_SENSOR = 3
} LightsMode;

struct State {
  int voltage; // VOLT * 10
  int sensor;  // Analog data - 0:1023 (1-1024?)
  bool motor_running;
  // char *ssid; // Wireless network ID
  QString ssid_string;
  LightsMode lights_mode = LIGHTS_OFF_MOTOR;
  unsigned long int motor_start_millis;
};

struct Settings {
  int motor_running_voltage_threshold = 0; // VOLT * 10
  int delay_lights_millis =
      3000; // Delay to turn lights on after the motor is running
  int sensor_threshold_to_turn_on = 0;
  int sensor_threshold_to_turn_off = 0;
  int wifi_connect_timeout =
      15000; // Timeout connectiong to WiFi and create self AP
  bool winter_mode = false;
  bool use_bufferd_udp = false;
  int localUdpPort = 0;
  int remoteUdpPort = 0;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void readPendingDatagrams();


private:
    Ui::MainWindow *ui;
    QUdpSocket* socket;
    QJsonObject json_object_received_state;
    QJsonObject json_object_command;
    QDateTime last_udp_status;
    State updated_state;
    Settings received_settings;

    void parseJson();
    void update_UI_from_json_status();
};
#endif // MAINWINDOW_H