#pragma once

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QMap>
#include <QList>
#include <QTimer>
#include <QResizeEvent>
#include <memory>

#include <rviz_common/visualization_manager.hpp>
#include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>
#include <controller_manager_msgs/srv/list_controllers.hpp>
#include <controller_manager_msgs/srv/switch_controller.hpp>
#include <rclcpp/rclcpp.hpp>

// Importamos la tarjeta
#include "../components/controller_card_widget.hpp"

class ControllersPanel : public QWidget {
  Q_OBJECT

public:
  explicit ControllersPanel(rviz_common::VisualizationManager * manager, QWidget *parent = nullptr);

  // Método para añadir manualmente o actualizar la lista de controladores
  void addOrUpdateController(const ControllerInfo & info);

protected:
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void checkControllersStatus();

private:
  void rearrangeGrid();
  void requestNodeParameters(const QString & ctrl_name, const QString & node_name, const QString & ctrl_state, const QString & ctrl_type);
  void handleControllerSwitch(const QString & controller_name, bool enable);
  void buildUI();
  rviz_common::VisualizationManager * manager_{nullptr};
  rclcpp::Node::SharedPtr ros_node_;

  // UI elements
  QLabel * title_label_{nullptr};
  QGridLayout * cards_layout_;
  QScrollArea * scroll_area_{nullptr};
  QWidget * scroll_container_{nullptr};
  QGridLayout * grid_layout_{nullptr};
  QTimer * timer_;

  // ROS 2 Clients
  rclcpp::Client<controller_manager_msgs::srv::ListControllers>::SharedPtr list_controllers_client_;
  rclcpp::Client<controller_manager_msgs::srv::SwitchController>::SharedPtr switch_controller_client_;

  // Gestión de estado y layout
  QTimer * status_timer_{nullptr};
  int current_calculated_columns_{-1};

  // Mapas y listas para trazabilidad de los widgets
  QMap<QString, ControllerCardWidget*> controllers_map_;
  QList<ControllerCardWidget*> controllers_list_;
  bool fetching_in_progress_{false};
};