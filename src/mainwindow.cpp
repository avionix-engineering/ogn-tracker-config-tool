#include "mainwindow.h"
#include "qdebug.h"
#include "ui_mainwindow.h"
#include <QComboBox>
#include <QTableWidgetItem>
#include <algorithm>

#include "serial.h"

int MainWindow::mapIntToHash(int val, QHash<int, QString> *hash) {
    // if number matches, nothing needs to be done
    if (hash->contains(val)) return val;

    // list ints from has keys and sort them
    QList<int> intList = hash->keys();
    std::sort(intList.begin(), intList.end());

    // if number is higher then highest key, return the highest key
    if (val > intList.last()) return intList.last();

    // find index of the highest key lower then provided value
    int index = 0;
    while (intList.count() - 1 > index && intList.at(index + 1) < val)
        index++;

    // return the key closest to the provided value
    if (val - intList.at(index) < intList.at(index + 1) - val)
        return intList.at(index);
    else
        return intList.at(index + 1);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // create value lists
    createValueList();

    // disable ui elements until connection is made
    ui->table->setColumnWidth(0, 150);
    ui->table->setEnabled(false);
    ui->refreshButton->setEnabled(false);
    ui->applyButton->setEnabled(false);
    updateSerialPortList();

    // connect timer (triggered every second) - check if any data was received. If not close serial port (the device was probably plugged out)
    connect(&timer, &QTimer::timeout, [&] {
        if (!serial.isConnected()) {
            updateSerialPortList();
            ui->statusBar->showMessage("Waiting for device...");
            serial.autoConnect();
        }
    });
    timer.start(500);

    // serial device disconnected - notify user, update connect button, force end recording
    connect(&serial, &Serial::disconnected, [&]() {
        ui->statusBar->showMessage("Connection closed");
        ui->table->setEnabled(false);
        ui->refreshButton->setEnabled(false);
        ui->applyButton->setEnabled(false);
    });

    // serial device connected - notify user, reset charts, update connect button
    connect(&serial, &Serial::connected, [&](QString name) {
        ui->statusBar->showMessage("Connected to OGN device found on " + name, 5000);
        disconnect(ui->serialPortList, &QComboBox::currentTextChanged, nullptr, nullptr);
        for (int i = 0; i < ui->serialPortList->count(); i++) {
            if (ui->serialPortList->itemText(i).contains(name)) {
                ui->serialPortList->setCurrentIndex(i);
                break;
            }
        }
        connect(ui->serialPortList, &QComboBox::currentTextChanged, this, &MainWindow::on_serialPortList_selected);
        updateDataTable();
        ui->refreshButton->setEnabled(true);
    });
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::createValueList() {
    aircraftTypesList.append("Unknown");
    aircraftTypesList.append("(Moto-)glider");
    aircraftTypesList.append("Tow plane");
    aircraftTypesList.append("Helicopter");
    aircraftTypesList.append("Parachute");
    aircraftTypesList.append("Drop plane");
    aircraftTypesList.append("hang-glider");
    aircraftTypesList.append("Para-glider");
    aircraftTypesList.append("Powered aircraft");
    aircraftTypesList.append("Jet aircraft");
    aircraftTypesList.append("UFO");
    aircraftTypesList.append("Balloon");
    aircraftTypesList.append("Airship");
    aircraftTypesList.append("UAV");
    aircraftTypesList.append("Ground support");
    aircraftTypesList.append("Static object");

    addressTypesList.append("Random");
    addressTypesList.append("ICAO");
    addressTypesList.append("FLARM");
    addressTypesList.append("OGN");

    freqTypesList.append("Automatic");
    freqTypesList.append("Europe");
    freqTypesList.append("USA/Canada");
    freqTypesList.append("South America /Australia");

    powerSettingsList.insert(10, "LOW");
    powerSettingsList.insert(14, "NORMAL");
    powerSettingsList.insert(22, "HIGH");
}

void MainWindow::createParamList() {
    // clear param list;
    foreach (Parameter *pointer, paramList)
        delete pointer;
    paramList.clear();

    // create selectors
    QComboBox *aircraftType = new QComboBox();
    aircraftType->addItems(aircraftTypesList);
    QComboBox *addrType = new QComboBox();
    addrType->addItems(addressTypesList);
    QComboBox *freqPlan = new QComboBox();
    freqPlan->addItems(freqTypesList);
    QComboBox *powerSel = new QComboBox();
    powerSel->addItems({"LOW", "NORMAL", "HIGH"});

    // create param list
    paramList.insert("Address", new Parameter("String"));
    paramList.insert("AddrType", new Parameter("Select", addrType));
    paramList.insert("AcftType", new Parameter("Select", aircraftType));
    paramList.insert("TxPower", new Parameter("StringHashInt+", powerSel, &powerSettingsList));
    paramList.insert("FreqPlan", new Parameter("SelectInt", freqPlan));
}

bool MainWindow::updateDataTable() {
    // clear table and disconnect cell changed signal
    ui->table->clear();
    ui->table->setRowCount(0);
    disconnect(ui->table, &QTableWidget::cellChanged, nullptr, nullptr);

    // disable buttons
    ui->table->setEnabled(false);
    ui->applyButton->setEnabled(false);
    ui->refreshButton->setEnabled(false);

    // update table headers
    ui->table->setHorizontalHeaderLabels({"Parameter", "Value"});

    // recreate param list (widgets were removed by table.clear)
    createParamList();

    QApplication::processEvents();

    // read buffer
    QByteArray *buffer = serial.readConfig();

    // if buffer empty, do nothing
    if (buffer == nullptr || buffer->length() < 10) {
        serial.disconnect();
        return false;
    }

    // get parameters list
    QStringList list = QString(*buffer).split('\n');

    // create table rows
    int row = 0;
    foreach (QString line, list) {
        // remove white characters
        line = line.remove(" ");

        // if line is correct:
        if (line.split("=").count() > 1) {
            // get paramater name and value
            QString param_s = line.split("=").at(0);
            QString val_s = line.split("=").at(1).split(";").at(0);

            if (advancedMode) {

                // load all params as strings

                // create param name item for table (non-editable)
                QTableWidgetItem *param = new QTableWidgetItem(param_s);
                param->setFlags(param->flags() ^ Qt::ItemIsEditable);

                // create param value item for table
                QTableWidgetItem *val = new QTableWidgetItem(val_s);

                // add item to table
                ui->table->setRowCount(row + 1);
                ui->table->setRowHeight(row, 20);
                ui->table->setItem(row, 1, val);
                ui->table->setItem(row, 0, param);

                // reset font (as non-bold)
                QFont f(font());
                f.setBold(false);

                // increment row number
                row++;

            } else if (paramList.contains(param_s)) {

                // load only params from list

                if (row > 0 && param_s == ui->table->item(0, 0)->text()) break;
                //qDebug() << param_s << val_s;

                // create param name item for table (non-editable)
                QTableWidgetItem *param = new QTableWidgetItem(param_s);
                param->setFlags(param->flags() ^ Qt::ItemIsEditable);

                // prepare table
                ui->table->setRowCount(row + 1);
                ui->table->setRowHeight(row, 20);

                // add item to table
                ui->table->setItem(row, 0, param);
                if (paramList.value(param_s)->widget != nullptr) {
                    // if list item has widget
                    ui->table->setCellWidget(row, 1, paramList.value(param_s)->widget);
                    if (paramList.value(param_s)->type == "Select") {
                        // type is select: widget is combobox, and data is hexodecimal number
                        QComboBox *selector = static_cast<QComboBox *>(paramList.value(param_s)->widget);
                        selector->setCurrentIndex(val_s.toUInt(nullptr, 16));

                        connect(selector, &QComboBox::currentTextChanged, [&, row](QString text) {
                            // change font as bold to mark the param as modified
                            QFont f(font());
                            f.setBold(true);
                            ui->table->item(row, 0)->setFont(f);
                        });
                    } else if (paramList.value(param_s)->type == "SelectInt") {
                        // type is select: widget is combobox, and data is decimal number
                        QComboBox *selector = static_cast<QComboBox *>(paramList.value(param_s)->widget);
                        selector->setCurrentIndex(val_s.toUInt());

                        connect(selector, &QComboBox::currentTextChanged, [&, row](QString text) {
                            // change font as bold to mark the param as modified
                            QFont f(font());
                            f.setBold(true);
                            ui->table->item(row, 0)->setFont(f);
                        });
                    } else if (paramList.value(param_s)->type == "StringHashInt+") {
                        // type is select: widget is combobox, data needs to be matched to hashmap
                        QComboBox *selector = static_cast<QComboBox *>(paramList.value(param_s)->widget);
                        int newVal = mapIntToHash(val_s.toInt(), paramList.value(param_s)->hash);
                        selector->setCurrentText(powerSettingsList.value(newVal));

                        connect(selector, &QComboBox::currentTextChanged, [&, row](QString text) {
                            // change font as bold to mark the param as modified
                            QFont f(font());
                            f.setBold(true);
                            ui->table->item(row, 0)->setFont(f);
                        });

                        // if value didn't match, it needs to be marked as modified
                        if (newVal != val_s.toInt()) selector->currentTextChanged(selector->currentText());
                    }
                } else
                    // item is just text
                    ui->table->setItem(row, 1, new QTableWidgetItem(val_s));

                // reset font (as non-bold)
                QFont f(font());
                f.setBold(false);

                // increment row number
                row++;
            }
        }
    }

    // reconnect cell changed signal
    connect(ui->table, &QTableWidget::cellChanged, this, &MainWindow::tableCellChanged);

    // reenable buttons
    ui->table->setEnabled(true);
    ui->applyButton->setEnabled(true);
    ui->refreshButton->setEnabled(true);
    return true;
}

void MainWindow::applyChanges() {
    ui->table->setEnabled(false);
    ui->statusBar->showMessage("Writing to device...");
    // for each parameter that has been modified
    for (int r = 0; r < ui->table->rowCount(); r++)
        if (ui->table->item(r, 0)->font().bold()) {
            // get param name and new value
            QByteArray param(ui->table->item(r, 0)->text().toUtf8());
            QByteArray val;

            // read data from table
            if (advancedMode || paramList.value(param)->type == "String") {
                // data is just text
                val = ui->table->item(r, 1)->text().toUtf8();
            } else if (paramList.value(param)->type == "Select") {
                // data needs to be read from combo box (selected index), and written in HEX, adding 0x at the beginning
                QComboBox *selector = static_cast<QComboBox *>(paramList.value(param)->widget);
                val = ("0x" + QString::number(selector->currentIndex(), 16).toUpper()).toUtf8();
            } else if (paramList.value(param)->type == "SelectInt") {
                // data needs to be read from combo box (selected index), and written as number
                QComboBox *selector = static_cast<QComboBox *>(paramList.value(param)->widget);
                val = QString::number(selector->currentIndex()).toUtf8();
            } else if (paramList.value(param)->type == "StringHashInt+") {
                // data needs to be read from hashtable matching combo box selection
                QComboBox *selector = static_cast<QComboBox *>(paramList.value(param)->widget);
                val = ("+" + QString::number(paramList.value(param)->hash->key(selector->currentText()))).toUtf8();
            }

            // send command to modify the parameter
            serial.send("$POGNS," + param + "=" + val + "\n");
        }

    // reload parameter list
    updateDataTable();
    ui->statusBar->showMessage("Changes saved");
}

void MainWindow::updateSerialPortList() {
    // disconnect port selector change event
    disconnect(ui->serialPortList, &QComboBox::currentTextChanged, nullptr, nullptr);
    ui->serialPortList->clear();

    // read device list
    QHash<QString, QString> devList = serial.listDevices();

    // add each device to the selector
    for (int i = 0; i < devList.count(); i++) {
        QString name = devList.keys().at(i);
        ui->serialPortList->addItem(name + " (" + devList.value(name) + ")");
    }
    // reconnect port selector change event
    connect(ui->serialPortList, &QComboBox::currentTextChanged, this, &MainWindow::on_serialPortList_selected);
}

void MainWindow::tableCellChanged(int row, int column) {
    // change font as bold to mark the param as modified
    QFont f(font());
    f.setBold(true);
    ui->table->item(row, column)->setFont(f);
    ui->table->item(row, 0)->setFont(f);
}

void MainWindow::on_refreshButton_clicked() {
    updateDataTable();
}

void MainWindow::on_applyButton_clicked() {
    applyChanges();
}

void MainWindow::on_serialPortList_selected(const QString &arg1) {
    // disconnect current device and connect selected one manually
    serial.disconnect();
    QString name = arg1.split(" (").at(0);
    serial.connect(name);
    qDebug() << name;
    serial.connected(name);
}

void MainWindow::on_buttonAdvanced_clicked(bool checked) {
    advancedMode = checked;
    updateDataTable();
}
