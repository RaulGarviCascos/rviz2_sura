#include "controllers_panel.hpp"
#include <rcl_interfaces/srv/list_parameters.hpp>
#include <rcl_interfaces/srv/get_parameters.hpp>
#include "../components/sura_controller_info.hpp"
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>

ControllersPanel::ControllersPanel(rviz_common::VisualizationManager * manager, QWidget * parent)
: QWidget(parent), manager_(manager), fetching_in_progress_(false)
{
  auto node_abs = manager_->getRosNodeAbstraction().lock();
  if (node_abs) {
    ros_node_ = node_abs->get_raw_node();
  } else {
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "ControllersPanel: No se pudo obtener el nodo de ROS 2.");
    return;
  }

  buildUI();

    // En ControllersPanel::ControllersPanel(...)
  QString robot_name = manager_->getRobotConfig().robot_name;

  std::string list_srv = "/" + robot_name.toStdString() + "/controller/controller_manager/list_controllers";
  std::string switch_srv = "/" + robot_name.toStdString() + "/controller/controller_manager/switch_controller";

  list_controllers_client_ = ros_node_->create_client<controller_manager_msgs::srv::ListControllers>(list_srv);
  switch_controller_client_ = ros_node_->create_client<controller_manager_msgs::srv::SwitchController>(switch_srv);
  // Timer para comprobar periódicamente el estado de los controladores
  timer_ = new QTimer(this);
  connect(timer_, &QTimer::timeout, this, &ControllersPanel::checkControllersStatus);
  timer_->start(2000); // Consulta cada 2 segundos

  // Disparar una primera comprobación
  checkControllersStatus();
}

void ControllersPanel::buildUI()
{
  QVBoxLayout * main_layout = new QVBoxLayout(this);

  QString robot_name = manager_->getRobotConfig().robot_name;
  title_label_ = new QLabel(tr("Controllers Panel - %1").arg(robot_name), this);
  title_label_->setAlignment(Qt::AlignCenter);
  title_label_->setStyleSheet("font-size: 18pt; font-weight: bold; margin: 10px;");
  main_layout->addWidget(title_label_);

  QScrollArea * scroll_area = new QScrollArea(this);
  scroll_area->setWidgetResizable(true);
  scroll_area->setFrameShape(QFrame::NoFrame);

  QWidget * scroll_content = new QWidget(scroll_area);
  cards_layout_ = new QGridLayout(scroll_content);
  cards_layout_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  scroll_content->setLayout(cards_layout_);

  scroll_area->setWidget(scroll_content);
  main_layout->addWidget(scroll_area);

  setLayout(main_layout);
}
void ControllersPanel::checkControllersStatus()
{
  // Evitar solapamiento de peticiones si una consulta previa sigue procesándose
  if (fetching_in_progress_) {
    return;
  }

  if (!list_controllers_client_ || !list_controllers_client_->service_is_ready()) {
    RCLCPP_WARN_THROTTLE(
      ros_node_->get_logger(), *ros_node_->get_clock(), 5000,
      "Esperando a que el servicio list_controllers esté disponible..."
    );
    return;
  }

  fetching_in_progress_ = true;
  auto request = std::make_shared<controller_manager_msgs::srv::ListControllers::Request>();

  list_controllers_client_->async_send_request(
    request, [this](rclcpp::Client<controller_manager_msgs::srv::ListControllers>::SharedFuture future) {
      fetching_in_progress_ = false;

      try {
        auto response = future.get();

        // 💡 Nos aseguramos de actualizar la interfaz de Qt de forma segura desde el hilo principal
        QMetaObject::invokeMethod(this, [this, response]() {
          for (const auto & ctrl : response->controller) {
            QString ctrl_name = QString::fromStdString(ctrl.name);
            QString ctrl_state = QString::fromStdString(ctrl.state);
            QString ctrl_type = QString::fromStdString(ctrl.type);

            if (controllers_map_.contains(ctrl_name)) {
              // 🔄 SI YA EXISTE: Simplemente actualizamos su estado (active/inactive)
              // Esto cambiará el LED, el botón y quitará el modo de carga (spinner)
              controllers_map_[ctrl_name]->updateState(ctrl_state);
            } 
            else {
              // 🆕 SI NO EXISTE: Pedimos parámetros para crear la tarjeta por primera vez
              QString robot_name = manager_->getRobotConfig().robot_name;
              QString full_node_path = QString("/%1/controller/%2").arg(robot_name, ctrl_name);
              
              if (!ctrl_name.contains("broadcaster", Qt::CaseInsensitive) &&
                  !ctrl_type.contains("broadcaster", Qt::CaseInsensitive)) 
              {
                requestNodeParameters(ctrl_name, full_node_path, ctrl_state, ctrl_type);
              }           
            }
          }
        }, Qt::QueuedConnection);

      } catch (const std::exception & e) {
        RCLCPP_ERROR(ros_node_->get_logger(), "Error en checkControllersStatus: %s", e.what());
      }
    });
}
void ControllersPanel::requestNodeParameters(
  const QString & ctrl_name, 
  const QString & node_path, 
  const QString & ctrl_state, 
  const QString & ctrl_type)
{
  std::string std_node_path = node_path.toStdString();
  auto list_client = ros_node_->create_client<rcl_interfaces::srv::ListParameters>(std_node_path + "/list_parameters");

  auto list_req = std::make_shared<rcl_interfaces::srv::ListParameters::Request>();
  list_req->depth = 0;

  list_client->async_send_request(
    list_req, [this, list_client, ctrl_name, ctrl_state, ctrl_type, std_node_path](
      rclcpp::Client<rcl_interfaces::srv::ListParameters>::SharedFuture list_future) {
      
      try {
        
        auto list_res = list_future.get();
        std::vector<std::string> filtered_names;

        for (const auto & name : list_res->result.names) {
          if (name != "robot_description" && name != "robot_description_semantic" && name != "use_sim_time") {
            filtered_names.push_back(name);
          }
        }

        if (filtered_names.empty()) {
          ControllerInfo info;
          info.name = ctrl_name;
          info.state = ctrl_state;
          info.type = ctrl_type;

          QMetaObject::invokeMethod(this, [this, info]() {
            addOrUpdateController(info);
          });
          return;
        }

        auto get_client = ros_node_->create_client<rcl_interfaces::srv::GetParameters>(std_node_path + "/get_parameters");
        auto get_req = std::make_shared<rcl_interfaces::srv::GetParameters::Request>();
        get_req->names = filtered_names;

        get_client->async_send_request(
          get_req, [this, get_client, ctrl_name, ctrl_state, ctrl_type, filtered_names](
            rclcpp::Client<rcl_interfaces::srv::GetParameters>::SharedFuture get_future) {

            try {
              auto get_res = get_future.get();
              ControllerInfo info;
              info.name = ctrl_name;
              info.state = ctrl_state;
              info.type = ctrl_type;

              for (size_t i = 0; i < filtered_names.size(); ++i) {
                if (get_res->values[i].type == rcl_interfaces::msg::ParameterType::PARAMETER_NOT_SET) continue;
                
                rclcpp::Parameter p(filtered_names[i], rclcpp::ParameterValue(get_res->values[i]));
                info.params.insert(QString::fromStdString(filtered_names[i]), QString::fromStdString(p.value_to_string()));
              }

              QMetaObject::invokeMethod(this, [this, info]() {
                addOrUpdateController(info);
              });
            } catch (...) {
              ControllerInfo info;
              info.name = ctrl_name;
              info.state = ctrl_state;
              info.type = ctrl_type;
              QMetaObject::invokeMethod(this, [this, info]() { addOrUpdateController(info); });
            }
          });

      } catch (const std::exception & e) {
        RCLCPP_ERROR(ros_node_->get_logger(), "Error leyendo parametros de %s: %s", std_node_path.c_str(), e.what());
        ControllerInfo info;
        info.name = ctrl_name;
        info.state = ctrl_state;
        info.type = ctrl_type;

        QMetaObject::invokeMethod(this, [this, info]() {
          addOrUpdateController(info);
        });
      }
    });
}

void ControllersPanel::addOrUpdateController(const ControllerInfo & info)
{

  if (controllers_map_.contains(info.name)) {
    controllers_map_[info.name]->updateState(info.state);
    return;
  }
  ControllerCardWidget * card = new ControllerCardWidget(info, ros_node_);
  
  // Conectamos la señal de la tarjeta al método del panel
  connect(card, &ControllerCardWidget::toggleRequested, 
          this, &ControllersPanel::handleControllerSwitch);

  int count = controllers_map_.size();
  int columns = 5; 
  int row = count / columns;
  int col = count % columns;

  cards_layout_->addWidget(card, row, col);
  controllers_map_.insert(info.name, card);
}


// LÓGICA DE ADAPTACIÓN RESPONSIVA EN GRID (Idéntica a SensorPanel)
void ControllersPanel::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  rearrangeGrid();
}

void ControllersPanel::rearrangeGrid()
{
  if (controllers_list_.isEmpty()) return;

  int available_width = scroll_area_->width();
  int card_width = 300; 
  int spacing = grid_layout_->spacing();
  int margins = grid_layout_->contentsMargins().left() + grid_layout_->contentsMargins().right();

  int cols = (available_width - margins + spacing) / (card_width + spacing);
  if (cols < 1) cols = 1;

  if (cols == current_calculated_columns_) return;
  current_calculated_columns_ = cols;

  QLayoutItem *child;
  while ((child = grid_layout_->takeAt(0)) != nullptr) {
    delete child;
  }

  int row = 0;
  int col = 0;
  for (ControllerCardWidget *card : controllers_list_) {
    grid_layout_->addWidget(card, row, col, Qt::AlignTop);
    
    col++;
    if (col >= cols) {
      col = 0;
      row++;
    }
  }

  grid_layout_->setRowStretch(row + 1, 1);
  for (int c = 0; c < cols; ++c) {
    grid_layout_->setColumnStretch(c, 1);
  }
}
void ControllersPanel::handleControllerSwitch(const QString & controller_name, bool enable)
{
  if (!switch_controller_client_) {
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "El cliente switch_controller no está inicializado");
    return;
  }
  if (!ros_node_) return;

  QString robot_name = manager_->getRobotConfig().robot_name;
  
  if (!switch_controller_client_->wait_for_service(std::chrono::milliseconds(300))) {
    RCLCPP_ERROR(ros_node_->get_logger(), "Servicio switch_controller no disponible");
    
    // Si falla la conexión, le quitamos el estado de carga a la tarjeta
    if (controllers_map_.contains(controller_name)) {
      controllers_map_[controller_name]->setLoading(false);
    }
    return;
  }

  auto req = std::make_shared<controller_manager_msgs::srv::SwitchController::Request>();
  
  if (enable) {
    req->activate_controllers.push_back(controller_name.toStdString());
  } else {
    req->deactivate_controllers.push_back(controller_name.toStdString());
  }


req->strictness = controller_manager_msgs::srv::SwitchController::Request::BEST_EFFORT;
req->activate_asap = true;

  switch_controller_client_->async_send_request(
    req, [controller_name, enable](rclcpp::Client<controller_manager_msgs::srv::SwitchController>::SharedFuture future) {
      try {
        auto response = future.get();
        if (response->ok) {
          RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Controlador %s %s con éxito", 
                      controller_name.toStdString().c_str(), enable ? "activados" : "desactivados");
        } else {
          RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Fallo al cambiar el estado del controlador %s", 
                       controller_name.toStdString().c_str());
        }
      } catch (const std::exception &e) {
        RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Excepción en switch_controller: %s", e.what());
      }
    });

}
