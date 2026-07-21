#ifndef SURA_URDF_PARSER_HPP
#define SURA_URDF_PARSER_HPP

#include <QString>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include "sura_sensors_info.hpp"
#include "sura_thrusters_info.hpp"

// Estructura para almacenar los datos limpios de cada sensor

class SuraUrdfParser {
public:
  SuraUrdfParser() = default;
  ~SuraUrdfParser() = default;

  // Carga y parsea el archivo URDF/Xacro
  bool parseUrdf(const QString &file_path);

  // Obtiene la lista de sensores detectados
  const QList<SuraSensorInfo>& getSensors() const { return sensors_; }
  const QList<SuraThrusterInfo>& getThrusters() const { return thrusters_; }

private:
  void parseRos2Control(QXmlStreamReader &xml);
  void parseSensor(QXmlStreamReader &xml);
  void parseThruster(QXmlStreamReader &xml);

  QList<SuraSensorInfo> sensors_;
  QList<SuraThrusterInfo> thrusters_;
};

#endif // SURA_URDF_PARSER_HPP