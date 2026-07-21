#pragma once 
#include <QString>
#include <QMap>
#include <QStringList>


struct SuraThrusterInfo {
  QString name;
  QStringList command_interfaces;
  QStringList state_interfaces;
  QMap<QString, QString> params;
};