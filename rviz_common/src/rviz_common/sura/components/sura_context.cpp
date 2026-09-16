#include "sura_context.hpp"

#include "rviz_common/ros_integration/ros_node_abstraction_iface.hpp"
#include "rviz_common/visualization_manager.hpp"

SuraContext::SuraContext(rviz_common::VisualizationManager * manager)
{
  if (manager == nullptr) {
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Cannot create SURA context without a manager.");
    return;
  }

  auto node_abstraction = manager->getRosNodeAbstraction().lock();
  if (!node_abstraction) {
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Cannot create SURA context without a ROS node.");
    return;
  }

  node_ = node_abstraction->get_raw_node();
  robot_config_ = manager->getRobotConfig();
}

bool SuraContext::isValid() const
{
  return static_cast<bool>(node_);
}

const rclcpp::Node::SharedPtr & SuraContext::node() const
{
  return node_;
}

const RobotConfig & SuraContext::robotConfig() const
{
  return robot_config_;
}

QString SuraContext::robotName() const
{
  return robot_config_.robot_name;
}

std::string SuraContext::controllerManagerService(const std::string & service) const
{
  return "/" + robotName().toStdString() + "/controller/controller_manager/" + service;
}

std::string SuraContext::controllerNodePath(const QString & controller_name) const
{
  return "/" + robotName().toStdString() + "/controller/" + controller_name.toStdString();
}

void SuraContext::updateRobotConfig(const RobotConfig & config)
{
  robot_config_ = config;
}
