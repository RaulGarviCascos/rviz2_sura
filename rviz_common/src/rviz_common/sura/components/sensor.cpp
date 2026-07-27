#include "sensor.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
Sensor::Sensor(const QString &sensor_name, QWidget *parent)
  : QWidget(parent), is_enabled_(false)
{
  sensor_name_=sensor_name;
  // Layout principal del Widget
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(0, 0, 0, 0);

  // Contenedor tipo "Tarjeta" 
  card_frame_ = new QFrame(this);
  card_frame_->setFrameShape(QFrame::StyledPanel);
  
  // 💡 CAMBIO CLAVE: Quitamos setFixedSize y permitimos tamaño dinámico
  card_frame_->setMinimumWidth(220);   // Ancho mínimo para que no se aplaste
  card_frame_->setMinimumHeight(180);  // Alto mínimo estético para sensores con pocos datos
  
  card_frame_->setStyleSheet(
    "QFrame {"
    "  background-color: #ffffff;"
    "  border: 1px solid #e1e8ed;" 
    "  border-radius: 12px;"       
    "}"
  );

  QVBoxLayout *card_layout = new QVBoxLayout(card_frame_);
  card_layout->setSpacing(12);
  card_layout->setContentsMargins(14, 14, 14, 14);

  // --- CABECERA: LED + Nombre ---
  QHBoxLayout *header_layout = new QHBoxLayout();
  header_layout->setSpacing(8);
  
  led_indicator_ = new QLabel(card_frame_);
  led_indicator_->setFixedSize(10, 10);
  updateLedStyle(false); 

  name_label_ = new QLabel(sensor_name, card_frame_);
  QFont name_font = name_label_->font();
  name_font.setBold(true);
  name_font.setPointSize(12); 
  name_label_->setFont(name_font);
  name_label_->setStyleSheet("border: none; color: #2c3e50;");
  name_label_->setWordWrap(true); // 💡 Si el nombre del sensor es largo, saltará de línea

  header_layout->addWidget(led_indicator_);
  header_layout->addWidget(name_label_);
  header_layout->addStretch(); 
  card_layout->addLayout(header_layout);

  // Línea divisoria
  QFrame *divider = new QFrame(card_frame_);
  divider->setFrameShape(QFrame::HLine);
  divider->setStyleSheet("background-color: #f1f2f6; max-height: 1px; border: none;");
  card_layout->addWidget(divider);

  // --- ZONA DE INFO ESTILO TABLA ZEBRA ---
  info_container_ = new QWidget(card_frame_);
  info_container_->setStyleSheet("border: none; background: transparent;");
  
  info_layout_ = new QVBoxLayout(info_container_);
  info_layout_->setContentsMargins(0, 4, 0, 4);
  info_layout_->setSpacing(6); 
  
  info_layout_->addStretch(1); // Mantiene las filas agrupadas arriba
  
  card_layout->addWidget(info_container_, 1); 

  main_layout->addWidget(card_frame_);
  setLayout(main_layout);

  // --- ZONA INFERIOR: BOTÓN DE ACCIÓN + SPINNER ---
  QHBoxLayout *button_layout = new QHBoxLayout();
  button_layout->setContentsMargins(0, 0, 0, 0);

  toggle_button_ = new SuraButton(SuraButton::Role::Default, tr("ON"), this);
  toggle_button_->setCheckable(true);
  updateButtonStyle();
  
  // Spinner / Circulito de carga
  spinner_ = new QProgressBar(this);
  spinner_->setRange(0, 0); // 💡 Rango (0,0) en Qt crea el modo indeterminado (animación continua)
  spinner_->setTextVisible(false);
  spinner_->setFixedHeight(6); // Fino y discreto
  spinner_->setStyleSheet(
    "QProgressBar {"
    "  border: none;"
    "  background-color: #f1f2f6;"
    "  border-radius: 3px;"
    "}"
    "QProgressBar::chunk {"
    "  background-color: #3498db;" // Azul de carga
    "  border-radius: 3px;"
    "}"
  );
  spinner_->hide(); // Oculto por defecto

  button_layout->addWidget(toggle_button_, 1);
  
  card_layout->addLayout(button_layout);
  card_layout->addWidget(spinner_); // Se coloca debajo del botón

  connect(toggle_button_, &SuraButton::clicked, this, &Sensor::onButtonClicked);
}
void Sensor::addInfoField(const QString &key, const QString &initial_value)
{
  if (fields_map_.contains(key)) return; 

  QFrame *row_frame = new QFrame(info_container_);
  row_frame->setFrameShape(QFrame::NoFrame);
  
  bool is_even = (fields_map_.size() % 2 == 0);
  if (is_even) {
    row_frame->setStyleSheet("QFrame { background-color: #d3d4d6; border-radius: 6px; }");
  } else {
    row_frame->setStyleSheet("QFrame { background-color: #ffffff; border-radius: 6px; }");
  }

  QHBoxLayout *row_layout = new QHBoxLayout(row_frame);
  row_layout->setContentsMargins(10, 8, 10, 8); 
  row_layout->setSpacing(8);

  // Etiqueta del campo (Clave)
  QLabel *label_key = new QLabel(key, row_frame);
  label_key->setStyleSheet(
    "font-size: 11px;"
    "font-weight: bold;"
    "color: #596566;" 
    "border: none;"
    "background: transparent;"
  );
  label_key->setWordWrap(true); 



  // Valor del campo
  QLabel *label_value = new QLabel(initial_value, row_frame);
  label_value->setStyleSheet(
    "font-size: 12px;"
    "font-weight: bold;"
    "color: #2c3e50;" 
    "border: none;"
    "background: transparent;"
  );
  label_value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  row_layout->addWidget(label_key, 1); // El '1' le da prioridad para expandirse si el texto es largo
  row_layout->addWidget(label_value, 0); // El '0' mantiene el valor compacto a la derecha

  int insert_index = info_layout_->count() - 1;
  info_layout_->insertWidget(insert_index, row_frame);

  fields_map_.insert(key, label_value); 

}

void Sensor::updateInfoField(const QString &key, const QString &value)
{
  if (fields_map_.contains(key)) {
    fields_map_[key]->setText(value);
  }
}

void Sensor::setLedActive(bool active)
{
  updateLedStyle(active);
}

bool Sensor::isSensorEnabled() const
{
  return is_enabled_;
}
void Sensor::setLoading(bool loading)
{
  is_loading_ = loading;
  if (loading) {
    toggle_button_->setEnabled(false); // Deshabilita clics
    toggle_button_->setText(tr("Processing..."));
    spinner_->show();
  } else {
    toggle_button_->setEnabled(true);  // Reactiva clics
    spinner_->hide();
    updateButtonStyle();               // Restaura texto ON / OFF
  }
}

void Sensor::onButtonClicked()
{
  // Al hacer clic, activamos el estado de carga y bloqueamos interacciones repetidas
  setLoading(true);
  emit sensorToggled(!is_enabled_);
}

void Sensor::setActive(bool active)
{
  // Cuando ROS 2 responde y confirma el estado real, quitamos el spinner
  setLoading(false);
  setLedActive(active);
  is_enabled_ = active;
  updateButtonStyle();
}

void Sensor::updateLedStyle(bool active)
{
  if (active) {
    led_indicator_->setStyleSheet("background-color: #2ecc71; border-radius: 5px; border: none;"); // Verde
  } else {
    led_indicator_->setStyleSheet("background-color: #e74c3c; border-radius: 5px; border: none;"); // Rojo
  }
}

void Sensor::updateButtonStyle()
{
  if (is_enabled_) {
    toggle_button_->setText("OFF");
    toggle_button_->setRole(SuraButton::Role::Danger);
  } else {
    toggle_button_->setText("ON");
    toggle_button_->setRole(SuraButton::Role::Default);
  }
}
void Sensor::setupRos(rclcpp::Node::SharedPtr node, const QString &robot_name, const QString &sensor_name, const QString &msg_type)
{
  if (!node) return;
  QString cleaned_sensor_name = sensor_name;
  cleaned_sensor_name.remove("_sensor");
  // Construimos el topic estándar: /robot_name/sensors/sensor_name
  std::string topic_name = QString("/%1/sensors/%2").arg(robot_name, cleaned_sensor_name).toStdString();

  // 💡 Helper lambda para actualizar el campo buscando entre varias claves candidatas 
  // (combina nombres de state_interface de Xacro con nombres legibles)
  auto updateAny = [this](const QStringList &candidate_keys, const QString &val) {
    for (const QString &key : candidate_keys) {
      if (fields_map_.contains(key)) {
        fields_map_[key]->setText(val);
        return;
      }
    }
  };

  // =========================================================================
  // ENCAPSULACIÓN Y DESPACHADOR DE MENSAJES (8 TIPOS)
  // =========================================================================

  // 1. IMU
  if (msg_type == "sensor_msgs/msg/Imu") {
    ros_sub_ = node->create_subscription<sensor_msgs::msg::Imu>(
      topic_name, rclcpp::SensorDataQoS(), [this, updateAny](const sensor_msgs::msg::Imu::SharedPtr msg) {
        QMetaObject::invokeMethod(this, [this, msg, updateAny]() {
          // Orientaciones
          updateAny({"orientation.x", "Roll", "roll"}, QString::number(msg->orientation.x, 'g', 4));
          updateAny({"orientation.y", "Pitch", "pitch"}, QString::number(msg->orientation.y, 'g', 4));
          updateAny({"orientation.z", "Yaw", "yaw"}, QString::number(msg->orientation.z, 'g', 4));
          updateAny({"orientation.w"}, QString::number(msg->orientation.w, 'g', 4));

          // Velocidad angular (Giroscopio)
          updateAny({"angular_velocity.x", "Gyro X"}, QString::number(msg->angular_velocity.x, 'g', 4)+" rad/s");
          updateAny({"angular_velocity.y", "Gyro Y"}, QString::number(msg->angular_velocity.y, 'g', 4)+" rad/s");
          updateAny({"angular_velocity.z", "Gyro Z"}, QString::number(msg->angular_velocity.z, 'g', 4)+" rad/s");

          // Aceleración lineal
          updateAny({"linear_acceleration.x", "Acc X", "acc_x"}, QString::number(msg->linear_acceleration.x, 'g', 4)+" m/s\u00B2");
          updateAny({"linear_acceleration.y", "Acc Y", "acc_y"}, QString::number(msg->linear_acceleration.y, 'g', 4)+" m/s\u00B2");
          updateAny({"linear_acceleration.z", "Acc Z", "acc_z"}, QString::number(msg->linear_acceleration.z, 'g', 4)+" m/s\u00B2");
        }, Qt::QueuedConnection);
      });
  }

  // 2. MAGNETIC FIELD
  else if (msg_type == "sensor_msgs/msg/MagneticField") {
    ros_sub_ = node->create_subscription<sensor_msgs::msg::MagneticField>(
      topic_name, rclcpp::SensorDataQoS(), [this, updateAny](const sensor_msgs::msg::MagneticField::SharedPtr msg) {
        QMetaObject::invokeMethod(this, [this, msg, updateAny]() {
          updateAny({"magnetic_field.x", "Mag X", "mag_x"}, QString::number(msg->magnetic_field.x, 'e', 4)+" T");
          updateAny({"magnetic_field.y", "Mag Y", "mag_y"}, QString::number(msg->magnetic_field.y, 'e', 4)+" T");
          updateAny({"magnetic_field.z", "Mag Z", "mag_z"}, QString::number(msg->magnetic_field.z, 'e', 4)+" T");
        }, Qt::QueuedConnection);
      });
  }

  // 3. FLUID PRESSURE (Profundidad / Presión agua)
  else if (msg_type == "sensor_msgs/msg/FluidPressure") {
    ros_sub_ = node->create_subscription<sensor_msgs::msg::FluidPressure>(
      topic_name, rclcpp::SensorDataQoS(), [this, updateAny](const sensor_msgs::msg::FluidPressure::SharedPtr msg) {
        QMetaObject::invokeMethod(this, [this, msg, updateAny]() {
          updateAny({"fluid_pressure", "Pressure", "pressure"}, QString::number(msg->fluid_pressure / 1000.0, 'f', 4) + " kPa");
        }, Qt::QueuedConnection);
      });
  }

  // 4. TWIST STAMPED (Velocidades medidas con timestamp)
  else if (msg_type == "geometry_msgs/msg/TwistWithCovarianceStamped") {
    
    QString clean_sensor_name = sensor_name;
    clean_sensor_name.remove("_sensor");

    std::string topic_name = QString("/%1/sensors/%2/twist_throttled")
                              .arg(robot_name, clean_sensor_name)
                              .toStdString();

    ros_sub_ = node->create_subscription<geometry_msgs::msg::TwistWithCovarianceStamped>(
      topic_name, rclcpp::SystemDefaultsQoS(), [this, updateAny, node](const geometry_msgs::msg::TwistWithCovarianceStamped::SharedPtr msg) {

        QMetaObject::invokeMethod(this, [this, msg, updateAny]() {
          updateAny({"twist.twist.linear.x", "twist.linear.x", "linear.x", "velocity.x", "linear_velocity.x", "Linear X", "vel_x", "x"}, 
                    QString::number(msg->twist.twist.linear.x, 'g', 4)+" m/s");
          updateAny({"twist.twist.linear.y", "twist.linear.y", "linear.y", "velocity.y", "linear_velocity.y", "Linear Y", "vel_y", "y"}, 
                    QString::number(msg->twist.twist.linear.y, 'g', 4)+" m/s");
          updateAny({"twist.twist.linear.z", "twist.linear.z", "linear.z", "velocity.z", "linear_velocity.z", "Linear Z", "vel_z", "z"}, 
                    QString::number(msg->twist.twist.linear.z, 'g', 4)+" m/s");
        }, Qt::QueuedConnection);
      });
  }

  // 5. RANGE (Sensores de ultrasonidos / Láser de distancia)
  else if (msg_type == "sensor_msgs/msg/Range") {
    ros_sub_ = node->create_subscription<sensor_msgs::msg::Range>(
      topic_name, rclcpp::SensorDataQoS(), [this, updateAny](const sensor_msgs::msg::Range::SharedPtr msg) {
        QMetaObject::invokeMethod(this, [this, msg, updateAny]() {
          updateAny({"range", "Distance", "distance"}, QString::number(msg->range, 'f', 4) + " m");
        }, Qt::QueuedConnection);
      });
  }

  // 6. BATTERY STATE
  else if (msg_type == "sensor_msgs/msg/BatteryState") {
    ros_sub_ = node->create_subscription<sensor_msgs::msg::BatteryState>(
      topic_name, rclcpp::SensorDataQoS(), [this, updateAny](const sensor_msgs::msg::BatteryState::SharedPtr msg) {
        QMetaObject::invokeMethod(this, [this, msg, updateAny]() {
          updateAny({"voltage", "Voltage"}, QString::number(msg->voltage, 'f', 4) + " V");
          updateAny({"current", "Current"}, QString::number(msg->current, 'f', 4) + " A");
          updateAny({"percentage", "Charge"}, QString::number(msg->percentage * 100.0, 'f', 0) + " %");
        }, Qt::QueuedConnection);
      });
  }

  // 7. SURA_MSGS: NAVIGATOR (Estructura compleja)
  else if (msg_type == "sura_msgs/msg/Navigator") {
    ros_sub_ = node->create_subscription<sura_msgs::msg::Navigator>(
      topic_name, rclcpp::SensorDataQoS(), [this, updateAny](const sura_msgs::msg::Navigator::SharedPtr msg) {
        QMetaObject::invokeMethod(this, [this, msg, updateAny]() {
          // Posición y Altitud
          updateAny({"position.position.x", "Pos X"}, QString::number(msg->position.position.x, 'f', 4));
          updateAny({"position.position.y", "Pos Y"}, QString::number(msg->position.position.y, 'f', 4));
          updateAny({"altitude", "Altitude"}, QString::number(msg->altitude, 'f', 2) + " m");
          
          // Orientación RPY (Roll, Pitch, Yaw)
          updateAny({"rpy.x", "Roll", "roll"}, QString::number(msg->rpy.x, 'f', 1));
          updateAny({"rpy.y", "Pitch", "pitch"}, QString::number(msg->rpy.y, 'f', 1));
          updateAny({"rpy.z", "Yaw", "yaw"}, QString::number(msg->rpy.z, 'f', 1));
          
          // Velocidad lineal en el cuerpo del vehículo (Body)
          updateAny({"body_velocity.linear.x", "Vel Body X"}, QString::number(msg->body_velocity.linear.x, 'f', 4));
          updateAny({"body_velocity.linear.y", "Vel Body Y"}, QString::number(msg->body_velocity.linear.y, 'f', 4));
          
          // Aceleraciones en el cuerpo (Body)
          updateAny({"body_acceleration.linear.x", "Acc Body X"}, QString::number(msg->body_acceleration.linear.x, 'f', 4));
        }, Qt::QueuedConnection);
      });
  }

  else if (msg_type == "std_msgs/msg/Bool") {
    ros_sub_ = node->create_subscription<std_msgs::msg::Bool>(
      topic_name, rclcpp::SensorDataQoS(), [this, updateAny](const std_msgs::msg::Bool::SharedPtr msg) {
        QMetaObject::invokeMethod(this, [this, msg, updateAny]() {
          // Devuelve "OK" o "LEAK!" (o "1" / "0") según el valor del booleano
          QString leak_status = msg->data ? "LEAK DETECTED" : "OK";
          
          updateAny({"leak", "leak_status", "water_leak", "state", "data"}, leak_status);
        }, Qt::QueuedConnection);
      });
  }

  // FALLBACK POR DEFECTO
  else {
    RCLCPP_WARN(node->get_logger(), "Tipo '%s' no mapeado explícitamente para el sensor '%s'.", 
                msg_type.toStdString().c_str(), sensor_name.toStdString().c_str());
  }
}

