#pragma once

#include <QWidget>
#include <QMap>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include "../components/sensor.hpp" // Asegúrate de que la ruta a tu cabecera sea correcta
#include <QResizeEvent>
#include <QList>

namespace rviz_common { class VisualizationManager; }


class SensorPanel : public QWidget
{
  Q_OBJECT

public:
  explicit SensorPanel(rviz_common::VisualizationManager *manager, QWidget *parent = nullptr);
  ~SensorPanel() override = default;

  // Crea y añade un nuevo sensor al panel automáticamente
  Sensor* addSensor(const QString &name);

  // Recupera un sensor por su nombre para poder actualizar sus campos desde fuera
  Sensor* getSensor(const QString &name) const;

signals:
  // Señal que se emite cuando CUALQUIER sensor cambia de estado (Encendido/Apagado)
  void sensorStateChanged(const QString &sensor_name, bool enabled);

private:
  rviz_common::VisualizationManager * manager_;
  void rearrangeGrid();                      
  int current_calculated_columns_ = -1;      
  QList<Sensor*> sensors_list_;
  QLabel *title_label_;
  QScrollArea *scroll_area_;
  QWidget *scroll_container_;
  QGridLayout *grid_layout_;

  // Mapa para gestionar los sensores de forma fácil usando su nombre como clave
  QMap<QString, Sensor*> sensors_map_;

  // Control de posición en la rejilla
  int current_row_ = 0;
  int current_column_ = 0;
  const int max_columns_ = 3; // Número de tarjetas de sensores por fila

  protected:
    void resizeEvent(QResizeEvent *event) override;
};