#ifndef RVIZ_COMMON__SURA__SURA_BF_HPP_
#define RVIZ_COMMON__SURA__SURA_BF_HPP_

#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "rviz_common/visibility_control.hpp"
#include "rviz_common/config.hpp"
#include "../../rviz_common/src/rviz_common/sura/components/sura_sensors_info.hpp"
#include "../../rviz_common/src/rviz_common/sura/components/sura_thrusters_info.hpp"
#include "../../rviz_common/src/rviz_common/sura/components/robot_config.hpp"
#include "rclcpp/rclcpp.hpp"


namespace rviz_common {
class VisualizationFrame;
class VisualizationManager;

}

class RVIZ_COMMON_PUBLIC SuraBF : public QObject
{
  Q_OBJECT
public:
explicit SuraBF(
  rviz_common::VisualizationFrame * frame, 
  QObject * parent = nullptr             
);
  virtual ~SuraBF();

  void initSuraUi(QWidget * rviz_render_panel);

  // Controla si las pestañas de SURA están visibles o no
  void setVisible(bool visible);
  QTabWidget * getWidget() const { return tab_widget_; }

signals:
  void tabChanged(int index);

public Q_SLOTS:
  void reloadXacro();
  
private:
  QWidget * createSensorsWidget();
  QWidget * createGraphicsWidget();
  QWidget * createActuatorsWidget();
  QWidget * createThrustersWidget();
  QWidget * createSettingsWidget();
  QWidget * createRviz3DWidget(QWidget * rviz_render_panel);
  void detachTab(int index);
  
  rviz_common::VisualizationFrame * frame_;
  rviz_common::VisualizationManager * manager_;

  QTabWidget * tab_widget_;

  QWidget * sensors_tab_;
  QWidget * graphics_tab_;
  QWidget * actuators_tab_;
  QWidget * thrusters_tab_;
  QWidget * settings_tab_;
  QWidget * rviz_3d_tab_;
  QString local_path_;
  QList<SuraSensorInfo> sensors_;
  QList<SuraThrusterInfo> thrusters_;
  bool correct_file_;
  RobotConfig r_config_;
  rclcpp::Node::SharedPtr ros_node_;
};

#endif  // RVIZ_COMMON__SURA__SURA_BF_HPP_