#ifndef RVIZ_COMMON__SURA__COMPONENTS__SURA_CONTEXT_HPP_
#define RVIZ_COMMON__SURA__COMPONENTS__SURA_CONTEXT_HPP_

#include <memory>
#include <string>

#include <QString>
#include <rclcpp/rclcpp.hpp>

#include "robot_config.hpp"

namespace rviz_common
{
class VisualizationManager;
}

class SuraContext
{
public:
  explicit SuraContext(rviz_common::VisualizationManager * manager);

  bool isValid() const;
  const rclcpp::Node::SharedPtr & node() const;
  const RobotConfig & robotConfig() const;
  QString robotName() const;

  std::string controllerManagerService(const std::string & service) const;
  std::string controllerNodePath(const QString & controller_name) const;

  void updateRobotConfig(const RobotConfig & config);

private:
  rclcpp::Node::SharedPtr node_;
  RobotConfig robot_config_;
};

#endif  // RVIZ_COMMON__SURA__COMPONENTS__SURA_CONTEXT_HPP_
