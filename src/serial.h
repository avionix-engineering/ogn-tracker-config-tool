#pragma once

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QTimer>

class Serial : public QObject {
    Q_OBJECT
    QSerialPort device;          // serial device
    QTimer timer;
    QThread *reader = nullptr;          // reader thread
    QByteArray buffer;                  // buffer to store bytes read from the device

    bool dataReceived = false;

    void readSerialLoop();

public:
    explicit Serial(QObject *parent = nullptr);

    bool autoConnect();
    void connect(QString name);
    void disconnect();
    bool isConnected();
    void send(QByteArray data);
    QByteArray *readConfig();

    static bool probeDevice(QString name);
    static QHash<QString, QString> listDevices();

signals:
    void connected(QString deviceName);
    void disconnected();
};
