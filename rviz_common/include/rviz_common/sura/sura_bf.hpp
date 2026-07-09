#ifndef RVIZ_COMMON__SURA__SURA_BF_HPP_
#define RVIZ_COMMON__SURA__SURA_BF_HPP_

#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "rviz_common/visibility_control.hpp"
#include "rviz_common/config.hpp"

namespace rviz_common {
class VisualizationFrame;
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
  
private:
  QWidget * createSensorsWidget();
  QWidget * createGraphicsWidget();
  QWidget * createThrustersWidget();
  QWidget * createSettingsWidget();
  QWidget * createRviz3DWidget(QWidget * rviz_render_panel);

  rviz_common::VisualizationFrame * frame_;
  QTabWidget * tab_widget_;

  QWidget * sensors_tab_;
  QWidget * graphics_tab_;
  QWidget * Thrusters_tab_;
  QWidget * settings_tab_;
  QWidget * rviz_3d_tab_;
};

#endif  // RVIZ_COMMON__SURA__SURA_BF_HPP_