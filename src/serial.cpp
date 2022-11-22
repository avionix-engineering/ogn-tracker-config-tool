#include "serial.h"
#include <QApplication>
#include <QDebug>
#include <QHash>
#include <QSerialPortInfo>
#include <QThread>
#include <QTimer>

Serial::Serial(QObject *parent) : QObject(parent) {
    // connect serial port readyRead (bytes received) - read serial loop (reads input byte by byte and sends data for processing)
    //QObject::connect(&device, &QSerialPort::readyRead, this, &Serial::readSerialLoop);

    // connect serial port aboutToClose - emit disconnected signal
    QObject::connect(&device, &QSerialPort::aboutToClose, [this]() {
        emit disconnected();
    });

    // connect timer (triggered every second) - check if any data was received. If not close serial port (the device was probably plugged out)
    QObject::connect(&timer, &QTimer::timeout, [&] {
        readSerialLoop();
        if (device.isOpen() && !dataReceived) disconnect();
        dataReceived = false;
    });
}

bool Serial::autoConnect() {
    // get the list of devices
    QHash<QString, QString> devList = Serial::listDevices();

    qDebug() << devList;

    // check each device on the list with device probe
    for (int i = 0; i < devList.count(); i++) {
        QString key = devList.keys().at(i);
        if (Serial::probeDevice(key)) {
            // if devie probe is succesfull connect to selected device
            connect(key);
            emit connected(key);
            return true;
        }
    }
    return false;
}

void Serial::connect(QString name) {
    qDebug() << "Connected to" << name;

    // close current connection (if exists)
    if (device.isOpen()) device.close();

    // setup device
    device.setPortName(name);
    device.setBaudRate(115200);

    // open device
    device.open(QIODevice::ReadWrite);
    device.setStopBits(QSerialPort::TwoStop);

    timer.start(1000);
}

void Serial::disconnect() {
    qDebug() << "Disconnected";
    // if device is open, it needs to be closed
    if (device.isOpen()) device.close();
    timer.stop();
    emit disconnected();
}

void Serial::readSerialLoop() {
    device.write("\x03");
    device.waitForReadyRead(100);
    dataReceived = device.canReadLine();
    while (device.bytesAvailable()) {
        // read byte from the device
        device.readAll();
        dataReceived = true;
    }
}

bool Serial::isConnected() {
    return device.isOpen();
}

void Serial::send(QByteArray data) {
    device.write(data);
    device.waitForReadyRead(1000);
    qDebug() << data;
}

QByteArray *Serial::readConfig() {
    if (!isConnected()) return nullptr;
    timer.stop();

    // clear buffers
    buffer.clear();
    device.clear();
    // send command to device
    device.write("\x03");

    // read config table
    QByteArray line;
    bool write = false;
    int err = 0;
    while (line.left(1) != "$" && !(write && !line.contains("="))) {
        // wait for ready read
        device.waitForReadyRead(1000);
        if (!device.canReadLine()) {
            QThread::msleep(100);
            QApplication::processEvents();
            err++;
            if (err >= 9) return nullptr;
        }
        while (device.canReadLine()) {
            // read until \n
            line = device.readLine();
            //if (write) qDebug() << line;
            // if line starting with $ found stop reading (table ended)
            if (line.left(1) == "$") break;
            if (write && !line.contains("=")) break;
            // ignore everything until line starting with Address found
            if (!write) write = line.left(7) == "Address";
            // add line to buffer
            if (write) buffer.append(line);
        }
        QThread::msleep(10);
    }
    dataReceived = true;

    timer.start(1000);
    // return pointer to buffer
    return &buffer;
}

bool Serial::probeDevice(QString name) {
    // setup serial device
    QSerialPort device;
    device.setPortName(name);
    device.setBaudRate(115200);
    QSerialPortInfo port(name);
    QStringList descr = {"Silicon Labs CP210x USB to UART Bridge", "CP2104 USB to UART Bridge Controller", "CP2102 USB to UART Bridge Controller"};
    // if device name matches, try to open device
    if (descr.contains(port.description()) && device.open(QIODevice::ReadWrite)) {
        // read fragment of data
        /*device.write("\x03");
        device.waitForReadyRead(10);
        QByteArray bytes = device.readAll();
        qDebug() << bytes;
        device.close();
        //QThread::msleep(300);
        // check if fragment ontains relevent signature
        return bytes.left(1) == "D";*/
        return true;
    }
    // could not open device
    return false;
}

QHash<QString, QString> Serial::listDevices() {
    // get the list of available serial ports
    QList<QSerialPortInfo> portList = QSerialPortInfo::availablePorts();
    QHash<QString, QString> devices;

    // fill the list with the devices: name-description
    foreach (QSerialPortInfo port, portList)
        devices[port.portName()] = port.description();

    return devices;
}
