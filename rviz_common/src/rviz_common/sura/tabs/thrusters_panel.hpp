#ifndef RVIZ_COMMON__SURA__TABS__THRUSTERS_PANEL_HPP_
#define RVIZ_COMMON__SURA__TABS__THRUSTERS_PANEL_HPP_

#include <QWidget>
#include "../components/thruster.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "rclcpp/rclcpp.hpp"
#include "controller_manager_msgs/srv/switch_controller.hpp"
#include <QScrollArea>
#include <QGridLayout>
#include "../components/sura_context.hpp"

class ThrustersPanel : public QWidget
{
  Q_OBJECT

public:
  Thruster* addThruster(const QString &name);
  Thruster* getThruster(const QString &name) const;

explicit ThrustersPanel(std::shared_ptr<SuraContext> context, QWidget *parent = nullptr);
~ThrustersPanel() override = default;

private slots:
  void onArmButtonClicked();


private:
  std::shared_ptr<SuraContext> context_;
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

#endif  // RVIZ_COMMON__SURA__TABS__THRUSTERS_PANEL_HPP_
