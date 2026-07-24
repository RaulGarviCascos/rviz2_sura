#include "sensor_panel.hpp"
#include <QVBoxLayout>
#include "rviz_common/visualization_manager.hpp"
#include "rviz_common/ros_integration/ros_node_abstraction_iface.hpp"
#include "controller_manager_msgs/srv/list_controllers.hpp"
#include "rclcpp/rclcpp.hpp"
#include <QTimer>

SensorPanel::SensorPanel(rviz_common::VisualizationManager * manager, QWidget *parent)
  : QWidget(parent), manager_(manager), current_calculated_columns_(-1) // Inicializamos a -1
{
  QString robot_name = manager_->getRobotConfig().robot_name;

  // Layout principal del panel
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(15, 15, 15, 15);
  main_layout->setSpacing(15);

  // Título del Panel de Sensores
  title_label_= new QLabel(tr("Sensors control panel - %1").arg(robot_name), this);
  title_label_->setAlignment(Qt::AlignCenter);
  QFont font = title_label_->font();
  font.setBold(true);
  font.setPointSize(30);
  title_label_->setFont(font);
  main_layout->addWidget(title_label_);

  // Contenedor interno que tendrá el Grid Layout
  scroll_container_ = new QWidget(this);
  grid_layout_ = new QGridLayout(scroll_container_);
  grid_layout_->setSpacing(15);
  grid_layout_->setContentsMargins(5, 5, 5, 5);
  scroll_container_->setLayout(grid_layout_);

  // Scroll Area responsive
  scroll_area_ = new QScrollArea(this);
  scroll_area_->setWidget(scroll_container_);
  scroll_area_->setWidgetResizable(true);
  scroll_area_->setFrameShape(QFrame::NoFrame);
  scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  
  main_layout->addWidget(scroll_area_);
  setLayout(main_layout);

  // =========================================================================
  // INTEGRACIÓN ROS 2: Cliente para consultar estado de controladores
  // =========================================================================
  auto ros_node = manager_->getRosNodeAbstraction().lock()->get_raw_node();

  std::string base_service_path = "/" + robot_name.toStdString() + "/controller/controller_manager/";

  list_controllers_client_ = ros_node->create_client<controller_manager_msgs::srv::ListControllers>(base_service_path + "list_controllers");
  switch_controller_client_ = ros_node->create_client<controller_manager_msgs::srv::SwitchController>(base_service_path + "switch_controller");


  // Timer para consultar periódicamente el estado de forma asíncrona (cada 2 segundos)
  status_timer_ = new QTimer(this);
  connect(status_timer_, &QTimer::timeout, this, &SensorPanel::checkControllersStatus);
  status_timer_->start(2000); 
}

Sensor* SensorPanel::addSensor(const SuraSensorInfo sensor)
{
  QString sensor_name=sensor.name;
  if (sensors_map_.contains(sensor_name)) {
    return sensors_map_[sensor_name]; 
  }

  // 1. Instanciamos la tarjeta del sensor
  Sensor *new_sensor = new Sensor(sensor_name, scroll_container_);
  
  // 💡 ESTABLECE LÍMITES: Así evitamos que se deformen en pantallas gigantes o se aplasten en las pequeñas
  new_sensor->setMinimumWidth(220); // Ancho mínimo recomendado para la tarjeta
  new_sensor->setMaximumWidth(350); // Ancho máximo para que no se estire de forma absurda

  sensors_map_.insert(sensor_name, new_sensor);
  sensors_info_map_.insert(sensor_name,sensor);
  sensors_list_.append(new_sensor); // Guardamos el orden físico de inserción

  // 2. Conectamos la señal
  connect(new_sensor, &Sensor::sensorToggled, this, [this, sensor_name](bool enabled) {
    handleSensorStateChanged(sensor_name, enabled);
  });

  // 3. Forzamos un recalculo del grid al añadir un elemento nuevo
  current_calculated_columns_ = -1;
  rearrangeGrid();

  return new_sensor;
}

Sensor* SensorPanel::getSensor(const QString &name) const
{
  return sensors_map_.value(name, nullptr);
}

// =========================================================================
// COMPROBACIÓN ASÍNCRONA DE CONTROLADORES / BROADCASTERS
// =========================================================================
void SensorPanel::checkControllersStatus()
{
  if (!list_controllers_client_ || !list_controllers_client_->service_is_ready()) {
    return;
  }

  auto request = std::make_shared<controller_manager_msgs::srv::ListControllers::Request>();

  list_controllers_client_->async_send_request(
    request, [this](rclcpp::Client<controller_manager_msgs::srv::ListControllers>::SharedFuture future) {
      try {
        auto response = future.get();
        
        QMap<QString, bool> active_controllers;
        for (const auto & ctrl : response->controller) {
          active_controllers.insert(
            QString::fromStdString(ctrl.name), 
            (ctrl.state == "active")
          );
        }

        for (Sensor *sensor : sensors_list_) {
          QString sensor_name = sensor->getSensorName();
          const SuraSensorInfo *sensor_info = getSuraSensorInfo(sensor_name);
          
          if (!sensor_info) continue;

          bool sensor_is_active = false;
          bool has_broadcasters = false;

          for (auto it = sensor_info->params.constBegin(); it != sensor_info->params.constEnd(); ++it) {
            if (it.key().startsWith("broadcaster")) {
              has_broadcasters = true;
              QString bc_name = it.value();

              if (active_controllers.value(bc_name, false)) {
                sensor_is_active = true; 
              }
            }
          }

          if (!has_broadcasters) {
            QString fallback_name = sensor_name;
            if (!fallback_name.endsWith("_broadcaster")) {
              fallback_name += "_broadcaster";
            }
            sensor_is_active = active_controllers.value(fallback_name, false);
          }

          sensor->setActive(sensor_is_active);
        }

      } catch (const std::exception & e) {
        RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Error al recibir respuesta de list_controllers: %s", e.what());
      }
    });
}
void SensorPanel::handleSensorStateChanged(const QString &sensor_name, bool enabled)
{
  if (!switch_controller_client_) {
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "El cliente switch_controller no está inicializado");
    return;
  }

  if (!switch_controller_client_->wait_for_service(std::chrono::milliseconds(200))) {
    RCLCPP_WARN(rclcpp::get_logger("rviz2"), "El servicio switch_controller no está disponible");
    return;
  }

  // Obtenemos la información guardada del sensor
  const SuraSensorInfo *sensor_info = getSuraSensorInfo(sensor_name);
  if (!sensor_info) {
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "No se encontró información para el sensor: %s", sensor_name.toStdString().c_str());
    return;
  }

  auto request = std::make_shared<controller_manager_msgs::srv::SwitchController::Request>();

  for (auto it = sensor_info->params.constBegin(); it != sensor_info->params.constEnd(); ++it) {
    if (it.key().startsWith("broadcaster")) {
      std::string controller_name = it.value().toStdString();

      if (!controller_name.empty()) {
        if (enabled) {
          request->activate_controllers.push_back(controller_name);
        } else {
          request->deactivate_controllers.push_back(controller_name);
        }
      }
    }
  }

  if (request->activate_controllers.empty() && request->deactivate_controllers.empty()) {
    QString fallback_name = sensor_name;
    if (!fallback_name.endsWith("_broadcaster")) {
      fallback_name += "_broadcaster";
    }

    std::string controller_name = fallback_name.toStdString();

    if (enabled) {
      request->activate_controllers.push_back(controller_name);
    } else {
      request->deactivate_controllers.push_back(controller_name);
    }
  }

  request->strictness = controller_manager_msgs::srv::SwitchController::Request::STRICT;
  request->activate_asap = true;

  switch_controller_client_->async_send_request(
    request, [sensor_name, enabled](rclcpp::Client<controller_manager_msgs::srv::SwitchController>::SharedFuture future) {
      try {
        auto response = future.get();
        if (response->ok) {
          RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Broadcasters de %s %s con éxito", 
                      sensor_name.toStdString().c_str(), enabled ? "activados" : "desactivados");
        } else {
          RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Fallo al cambiar el estado de los broadcasters de %s", 
                       sensor_name.toStdString().c_str());
        }
      } catch (const std::exception &e) {
        RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Excepción en switch_controller: %s", e.what());
      }
    });
}

// =========================================================================
// LÓGICA DE ADAPTACIÓN DINÁMICA
// =========================================================================

void SensorPanel::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  rearrangeGrid(); // Cada vez que RViz o la ventana cambie de tamaño, recalculamos
}

void SensorPanel::rearrangeGrid()
{
  if (sensors_list_.isEmpty()) return;

  // 1. Calculamos el espacio real disponible en el scroll area
  int available_width = scroll_area_->width();
  
  // Anchura objetivo de cada tarjeta + márgenes y espaciados del layout
  int card_width = 220; 
  int spacing = grid_layout_->spacing();
  int margins = grid_layout_->contentsMargins().left() + grid_layout_->contentsMargins().right();

  // 2. Determinamos cuántas columnas caben físicamente en este momento
  int cols = (available_width - margins + spacing) / (card_width + spacing);
  if (cols < 1) cols = 1;

  // Evitamos parpadeos y cálculos innecesarios si el número de columnas no ha cambiado
  if (cols == current_calculated_columns_) return;
  current_calculated_columns_ = cols;

  // 3. Vaciamos el layout actual de forma segura (sin destruir los widgets de las tarjetas)
  QLayoutItem *child;
  while ((child = grid_layout_->takeAt(0)) != nullptr) {
    delete child; // Borra el contenedor del layout item, pero mantiene vivo el widget dentro
  }

  // 4. Distribuimos los sensores secuencialmente en el nuevo número de columnas
  int row = 0;
  int col = 0;
  for (Sensor *sensor : sensors_list_) {
    grid_layout_->addWidget(sensor, row, col, Qt::AlignTop);
    
    col++;
    if (col >= cols) {
      col = 0;
      row++;
    }
  }

  // 5. Empujamos todo hacia arriba para que no floten verticalmente si hay pocas filas
  grid_layout_->setRowStretch(row + 1, 1);
  
  // Forzamos a que todas las columnas tengan el mismo peso de estiramiento
  for (int c = 0; c < cols; ++c) {
    grid_layout_->setColumnStretch(c, 1);
  }
}