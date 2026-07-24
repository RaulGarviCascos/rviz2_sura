#include "sura_urdf_parser.hpp"

// Método principal para cargar y parsear el archivo URDF/Xacro
bool SuraUrdfParser::parseUrdf(const QString &file_path)
{
  QFile file(file_path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "[SuraUrdfParser] No se pudo abrir el archivo:" << file_path;
    return false;
  }

  sensors_.clear();
  thrusters_.clear(); 
  
  QXmlStreamReader xml(&file);

  while (!xml.atEnd() && !xml.hasError()) {
    QXmlStreamReader::TokenType token = xml.readNext();
    
    if (token == QXmlStreamReader::StartElement) {
      if (xml.name().toString() == "ros2_control") {
        parseRos2Control(xml);
      }
    }
  }

  if (xml.hasError()) {
    qWarning() << "[SuraUrdfParser] Error parseando XML:" << xml.errorString();
    return false;
  }

  return true;
}

// Procesa el bloque interno de <ros2_control>
void SuraUrdfParser::parseRos2Control(QXmlStreamReader &xml)
{
  // Seguimos leyendo hasta encontrar el cierre </ros2_control>
  while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name().toString() == "ros2_control")) {
    QXmlStreamReader::TokenType token = xml.readNext();
    
    if (token == QXmlStreamReader::StartElement) {
      if (xml.name().toString() == "sensor") {
        parseSensor(xml);
      }
      else if (xml.name().toString() == "joint") {
        QString joint_name = xml.attributes().value("name").toString();
        if (joint_name.contains("Thruster", Qt::CaseInsensitive)) {
          parseThruster(xml);
        }
      }
    }
   
  }
}

// Extrae la información de cada etiqueta <sensor>
void SuraUrdfParser::parseSensor(QXmlStreamReader &xml)
{
  SuraSensorInfo sensor;
  sensor.name = xml.attributes().value("name").toString();
  int broadcasters_count = 0;
  // Seguimos leyendo hasta encontrar el cierre </sensor>
  while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name().toString() == "sensor")) {
    QXmlStreamReader::TokenType token = xml.readNext();
    
    if (token == QXmlStreamReader::StartElement) {
      QString tag_name = xml.name().toString();
      
      if (tag_name == "param") {
        QString param_name = xml.attributes().value("name").toString();
        QString param_value = xml.readElementText(); 

        // Si el parámetro se llama "broadcaster", le asignamos un índice incremental
        if (param_name == "broadcaster") {
          param_name = QString("broadcaster%1").arg(broadcasters_count);
          broadcasters_count++;
        }

        sensor.params.insert(param_name, param_value);
      }
      else if (tag_name == "state_interface") {
        QString interface_name = xml.attributes().value("name").toString();
        sensor.state_interfaces.append(interface_name);
      }
    }
  }

  sensors_.append(sensor);
}

// Extrae la información de cada etiqueta <thruster>

void SuraUrdfParser::parseThruster(QXmlStreamReader &xml)
{
  SuraThrusterInfo thruster;
  // Guardamos el nombre del joint (motor)
  thruster.name = xml.attributes().value("name").toString();
 
  QStringList parts = thruster.name.split('/');

  thruster.name = parts[1];
  // thruster.name.remove("Thruster_");
  // Seguimos leyendo hasta encontrar el cierre </joint>
  while (!xml.atEnd() && !(xml.tokenType() == QXmlStreamReader::EndElement && xml.name().toString() == "joint")) {
    QXmlStreamReader::TokenType token = xml.readNext();
    
    if (token == QXmlStreamReader::StartElement) {
      QString tag_name = xml.name().toString();
      
      if (tag_name == "param") {
        QString param_name = xml.attributes().value("name").toString();
        QString param_value = xml.readElementText(); 
        thruster.params.insert(param_name, param_value);
      } 
      else if (tag_name == "state_interface") {
        QString interface_name = xml.attributes().value("name").toString();
        thruster.state_interfaces.append(interface_name);
      }
      else if (tag_name == "command_interface") { 
        QString interface_name = xml.attributes().value("name").toString();
        thruster.command_interfaces.append(interface_name);
      }
    }
  }

  thrusters_.append(thruster);
}

