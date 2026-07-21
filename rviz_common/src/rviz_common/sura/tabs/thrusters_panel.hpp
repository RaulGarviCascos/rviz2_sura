#ifndef Thrusters_PANEL_HPP
#define Thrusters_PANEL_HPP

#include <QWidget>
#include "../components/thruster.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "rclcpp/rclcpp.hpp"
#include "controller_manager_msgs/srv/switch_controller.hpp"
#include <QScrollArea> 
#include <QGridLayout>

namespace rviz_common { class VisualizationManager; }

class ThrustersPanel : public QWidget
{
  Q_OBJECT

public:
  Thruster* addThruster(const QString &name);
  Thruster* getThruster(const QString &name) const;

explicit ThrustersPanel(rviz_common::VisualizationManager *manager, QWidget *parent = nullptr);
~ThrustersPanel() override = default;

private slots:
  void onArmButtonClicked();


private:
  rviz_common::VisualizationManager * manager_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr thruster_pub_;
  rclcpp::Client<controller_manager_msgs::srv::SwitchController>::SharedPtr switch_controller_client_;
  SuraButton *btn_save_;
  SuraButton *btn_arm_;
  bool is_armed_ = false;
  void updateArmed(); 
  QTimer *panel_timer_;  
  QGridLayout *thrusters_grid_;
  QWidget *scroll_widget_;
  QScrollArea *scroll_area_;
  int current_columns_ = 4; 
  int current_calculated_columns_ = -1;      
  QList<Thruster*> thruster_list_;
  QMap<QString, Thruster*> thruster_map_;

protected:
  void resizeEvent(QResizeEvent *event) override;
  

};

#endif // ThrusterS_PANEL_HPP