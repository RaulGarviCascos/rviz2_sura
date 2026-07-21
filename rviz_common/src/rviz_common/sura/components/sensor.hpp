#pragma once

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QHash>
#include <QString>
#include "sura_button.hpp"
#include <rclcpp/rclcpp.hpp>

#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/magnetic_field.hpp>
#include <sensor_msgs/msg/fluid_pressure.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <sensor_msgs/msg/range.hpp>
#include <sensor_msgs/msg/battery_state.hpp>
#include "sura_msgs/msg/navigator.hpp"
#include "sura_msgs/msg/leak_sensor.hpp"
#include <geometry_msgs/msg/twist_with_covariance_stamped.hpp>
#include <std_msgs/msg/bool.hpp>

class Sensor : public QWidget
{
  Q_OBJECT

public:
  explicit Sensor(const QString &sensor_name, QWidget *parent = nullptr);
  virtual ~Sensor() = default;

  // Enciende/Apaga visualmente el LED indicador
  void setLedActive(bool active);

  // Añade un nuevo campo de información (ej: "Profundidad", "0.0 m")
  void addInfoField(const QString &key, const QString &initial_value = "-");

  // Actualiza el valor de un campo existente en tiempo real
  void updateInfoField(const QString &key, const QString &value);

  // Devuelve si el botón del sensor está activado o no
  bool isSensorEnabled() const;

  void setupRos(rclcpp::Node::SharedPtr node, const QString &robot_name, const QString &sensor_name, const QString &msg_type);

signals:
  // Se emite cuando el usuario pulsa el botón de encender/apagar
  void sensorToggled(bool enabled);

private slots:
  void onButtonClicked();

private:
  bool is_enabled_;
  
  // Elementos UI
  QFrame *card_frame_;
  QLabel *led_indicator_;
  QLabel *name_label_;
  SuraButton *toggle_button_;
  
  // Zona de información adaptable
  QWidget *info_container_;
  QVBoxLayout *info_layout_;
  
  // Mapa para encontrar y actualizar los valores rápidamente por su clave (Key)
  QHash<QString, QLabel*> fields_map_;

  // Métodos de estilo
  void updateLedStyle(bool active);
  void updateButtonStyle();
  rclcpp::SubscriptionBase::SharedPtr ros_sub_;
};