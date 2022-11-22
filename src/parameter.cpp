#include "parameter.h"

Parameter::Parameter(QString type, QWidget *widget, QHash<int, QString> *hash) {
    this->type = type;
    this->widget = widget;
    this->hash = hash;
}
