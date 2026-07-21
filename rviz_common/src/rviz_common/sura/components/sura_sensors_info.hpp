#pragma once 
#include <QString>
#include <QMap>
#include <QStringList>


struct SuraSensorInfo {
  QString name;
  QMap<QString, QString> params;
  QStringList state_interfaces;
};
