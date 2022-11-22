#pragma once

#include "parameter.h"
#include "serial.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
    Serial serial;                                  // serial port object
    QTimer timer;                                   // times (used to rerty connection
    bool advancedMode = false;                      // advanced mode - display all params as strings
    QHash<QString, Parameter *> paramList;          // list of parameters that should be read from the device
                                                    //
    QList<QString> aircraftTypesList;               // list of aircraft types           (index -> type name)
    QList<QString> addressTypesList;                // list of address types            (index -> type name)
    QList<QString> freqTypesList;                   // list of freq types / regions     (index -> region)
    QHash<int, QString> powerSettingsList;          // list of power settings           (power -> label)

    /** Matches proveded values with provided hash map keys, and returns the closest key
     * @param val  - proveded value
     * @param hash - hash map
     * @returns kay closest to the provided value
     */
    int mapIntToHash(int val, QHash<int, QString> *hash);

    /** Fills values lists (needs to be executed only once) */
    void createValueList();

    /** Creates list of parameters, along with widgets used to change the values. */
    void createParamList();

    /** Updates table - reads data from currently connected device and fills up the table */
    bool updateDataTable();

    /** Reads all changed parameters from the table and sends commands to the device */
    void applyChanges();

    /** Updates contents of combobox for port selection */
    void updateSerialPortList();

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void tableCellChanged(int row, int column);
    void on_refreshButton_clicked();
    void on_applyButton_clicked();
    void on_serialPortList_selected(const QString &arg1);

    void on_buttonAdvanced_clicked(bool checked);

private:
    Ui::MainWindow *ui;
};
