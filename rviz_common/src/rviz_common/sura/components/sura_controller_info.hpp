#pragma once
#include <string>
#include <unordered_map>
#include <QString>
#include <QMap>

struct ControllerInfo {
  QString name;
  QString type;
  QString state;
  QMap<QString, QString> params;
};