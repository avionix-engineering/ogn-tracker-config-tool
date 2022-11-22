#pragma once
#include <QString>
#include <QWidget>

class Parameter {
public:
    QString type;
    QWidget *widget = nullptr;
    QHash<int, QString> *hash;

    Parameter(QString type, QWidget *widget = nullptr, QHash<int, QString> *hash = nullptr);
};
