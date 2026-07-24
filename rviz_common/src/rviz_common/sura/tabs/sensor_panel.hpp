#pragma once

#include <QWidget>
#include <QMap>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include "../components/sensor.hpp" // Asegúrate de que la ruta a tu cabecera sea correcta
#include <QResizeEvent>
#include <QList>
#include "controller_manager_msgs/srv/list_controllers.hpp"
#include "controller_manager_msgs/srv/switch_controller.hpp"
#include "rclcpp/rclcpp.hpp"
#include <QTimer>
#include "../components/sura_sensors_info.hpp"

namespace rviz_common { class VisualizationManager; }


class SensorPanel : public QWidget
{
  Q_OBJECT

public:
  explicit SensorPanel(rviz_common::VisualizationManager *manager, QWidget *parent = nullptr);
  ~SensorPanel() override = default;

  // Crea y añade un nuevo sensor al panel automáticamente
  Sensor* addSensor(const SuraSensorInfo sensor);

  // Recupera un sensor por su nombre para poder actualizar sus campos desde fuera
  Sensor* getSensor(const QString &name) const;

  const SuraSensorInfo* getSuraSensorInfo(const QString &name) const {
      auto it = sensors_info_map_.constFind(name);
      return (it != sensors_info_map_.constEnd()) ? &it.value() : nullptr;
    }

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
  QMap<QString, SuraSensorInfo> sensors_info_map_;

  // Control de posición en la rejilla
  int current_row_ = 0;
  int current_column_ = 0;
  const int max_columns_ = 3; // Número de tarjetas de sensores por fila
  QTimer *status_timer_{nullptr};
  rclcpp::Client<controller_manager_msgs::srv::ListControllers>::SharedPtr list_controllers_client_{nullptr};
  rclcpp::Client<controller_manager_msgs::srv::SwitchController>::SharedPtr switch_controller_client_{nullptr};
  void handleSensorStateChanged(const QString &sensor_name, bool enabled);
  private slots:
  void checkControllersStatus();

  protected:
    void resizeEvent(QResizeEvent *event) override;
};