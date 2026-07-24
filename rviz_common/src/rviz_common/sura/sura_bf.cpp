#include "rviz_common/sura/sura_bf.hpp"
#include "rviz_common/visualization_frame.hpp"
#include "rviz_common/visualization_manager.hpp"
#include "rclcpp/rclcpp.hpp"
#include "tabs/thrusters_panel.hpp"
#include "tabs/settings_panel.hpp"
#include "tabs/sensor_panel.hpp"
#include "components/sura_urdf_parser.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include <filesystem>
#include <QDir>
#include <QTabBar>
#include <QDialog>

SuraBF::SuraBF(rviz_common::VisualizationFrame * frame,  QObject * parent)
: QObject(parent),
  frame_(frame),
  tab_widget_(nullptr),
  sensors_tab_(nullptr),
  graphics_tab_(nullptr),
  thrusters_tab_(nullptr),
  settings_tab_(nullptr),
  rviz_3d_tab_(nullptr),
  correct_file_(false){
  manager_=frame_->getManager();
}

SuraBF::~SuraBF()
{
  if (tab_widget_ && !tab_widget_->parentWidget()) {
    delete tab_widget_;
  }
}

void SuraBF::initSuraUi(QWidget * rviz_render_panel)
{
  if (tab_widget_) return;
  r_config_ = manager_->getRobotConfig();

  tab_widget_ = new QTabWidget(frame_);
  QString target_dir = QDir::homePath() + "/.cirtesu/xacros";
  local_path_ = target_dir + "/" + r_config_.robot_name + ".urdf.xacro";
  SuraUrdfParser parser;
  if (parser.parseUrdf(local_path_)) {
    sensors_=parser.getSensors();
    thrusters_=parser.getThrusters();
    correct_file_ = true;
  }else{
    correct_file_ = false;
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Could not parse the file: %s", local_path_.toStdString().c_str());
  }

  sensors_tab_ = createSensorsWidget();
  graphics_tab_ = createGraphicsWidget();
  thrusters_tab_ = createThrustersWidget();
  settings_tab_ = createSettingsWidget();
  rviz_3d_tab_ = createRviz3DWidget(rviz_render_panel);

  if (SettingsPanel * settings_panel_ptr = qobject_cast<SettingsPanel*>(settings_tab_)) {
    connect(settings_panel_ptr, &SettingsPanel::xacroUpdated, this, &SuraBF::reloadXacro);
  }

  tab_widget_->addTab(sensors_tab_, tr("Sensors"));
  tab_widget_->addTab(graphics_tab_, tr("Graphics"));
  tab_widget_->addTab(thrusters_tab_, tr("Thursters"));
  tab_widget_->addTab(settings_tab_, tr("Settings"));
  tab_widget_->addTab(rviz_3d_tab_, tr("3D View"));
  connect(tab_widget_, &QTabWidget::currentChanged, this, [this](int index) {
    emit tabChanged(index);
  });
  tab_widget_->setVisible(true);
  connect(tab_widget_->tabBar(), &QTabBar::tabBarDoubleClicked, this, &SuraBF::detachTab);
}

void SuraBF::setVisible(bool visible)
{
  if (tab_widget_) {
    tab_widget_->setVisible(visible);
  }
}

QWidget * SuraBF::createSensorsWidget()
{
  SensorPanel * sensor_panel = new SensorPanel(frame_->getManager(),tab_widget_);
  auto ros_node = manager_->getRosNodeAbstraction().lock()->get_raw_node();
  if(correct_file_){
    for (const SuraSensorInfo &sensor_info : sensors_) {
      Sensor * ui_card = sensor_panel->addSensor(sensor_info);
      for (const QString &state_name : sensor_info.state_interfaces) {
        if (state_name.contains("sample_time")) continue;
        ui_card->addInfoField(state_name, "0.0");
      }
      ui_card->setupRos(
        ros_node,
        r_config_.robot_name,
        sensor_info.name,
        sensor_info.params["msg_type"]
      );
    }
  } else  {
    SuraSensorInfo new_info;
    new_info.name = "DVL (Fallback)";

    Sensor * fallback = sensor_panel->addSensor(new_info);
    fallback->addInfoField("Vel X", "0.0");
    fallback->addInfoField("Vel Y", "0.0");

    if (QVBoxLayout *existing_layout = qobject_cast<QVBoxLayout*>(sensor_panel->layout())) {
      QLabel * label = new QLabel(
        tr("⚠️ Invalid or missing description file:\n%1").arg(local_path_), 
        sensor_panel
      );
      label->setAlignment(Qt::AlignCenter);
      label->setStyleSheet("color: #e74c3c; font-weight: bold; padding: 10px; background-color: #fadbd8; border-radius: 5px;");
      existing_layout->insertWidget(1, label);
    }
  }

  return sensor_panel;
}

QWidget * SuraBF::createGraphicsWidget()
{
  QWidget * widget = new QWidget(tab_widget_);
  QVBoxLayout * layout = new QVBoxLayout(widget);

  QLabel * label = new QLabel(tr("Gráficas"), widget);
  label->setAlignment(Qt::AlignCenter);

  layout->addWidget(label);
  widget->setLayout(layout);
  return widget;
}

QWidget * SuraBF::createThrustersWidget()
{
  ThrustersPanel * thrusters_panel = new ThrustersPanel(frame_->getManager(), tab_widget_);
  if(correct_file_){
    for (const SuraThrusterInfo &thrusterInfo : thrusters_) {
      thrusters_panel->addThruster(thrusterInfo.name);
    }
  } else {
    thrusters_panel->addThruster("Thruster (Fallback)");

    if (QVBoxLayout *existing_layout = qobject_cast<QVBoxLayout*>(thrusters_panel->layout())) {
      QLabel * label = new QLabel(
        tr("⚠️ Invalid or missing description file:\n%1").arg(local_path_), 
        thrusters_panel
      );
      label->setAlignment(Qt::AlignCenter);
      label->setStyleSheet("color: #e74c3c; font-weight: bold; padding: 10px; background-color: #fadbd8; border-radius: 5px;");
      existing_layout->insertWidget(1, label);
    }
  }
  return thrusters_panel;
}

QWidget * SuraBF::createSettingsWidget()
{
  SettingsPanel * settingsPanel = new SettingsPanel(frame_, tab_widget_);
  return settingsPanel;
}

QWidget * SuraBF::createRviz3DWidget(QWidget * rviz_render_panel)
{
  QWidget * widget = new QWidget(tab_widget_);
  QVBoxLayout * layout = new QVBoxLayout(widget);
  layout->setContentsMargins(0, 0, 0, 0); 

  if (rviz_render_panel) {
    rviz_render_panel->setParent(widget);
    layout->addWidget(rviz_render_panel);
  } else {
    QLabel * label = new QLabel(tr("Error: No se ha podido cargar la Vista 3D nativa"), widget);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
  }

  widget->setLayout(layout);
  return widget;
}

void SuraBF::reloadXacro()
{
  r_config_ = manager_->getRobotConfig();
  QString target_dir = QDir::homePath() + "/.cirtesu/xacros";
  local_path_ = target_dir + "/" + r_config_.robot_name + ".urdf.xacro";

  SuraUrdfParser parser;
  if (parser.parseUrdf(local_path_)) {
    sensors_ = parser.getSensors();
    thrusters_ = parser.getThrusters();
    correct_file_ = true;
    RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Xacro reloaded successfully: %s", local_path_.toStdString().c_str());
  } else {
    correct_file_ = false;
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Could not parse the reloaded file: %s", local_path_.toStdString().c_str());
  }

  if (tab_widget_ && sensors_tab_) {
    int index = tab_widget_->indexOf(sensors_tab_);
    
    QWidget * new_sensors_tab = createSensorsWidget();
    
    tab_widget_->removeTab(index);
    sensors_tab_->deleteLater(); 
    sensors_tab_ = new_sensors_tab;
    
    tab_widget_->insertTab(index, sensors_tab_, tr("Sensors"));
  }
}
void SuraBF::detachTab(int index)
{
  if (index < 0) return;

  QString title = tab_widget_->tabText(index);
  QWidget *content_widget = tab_widget_->widget(index);

  if (!content_widget) return;

  QDialog *float_window = new QDialog(frame_);
  float_window->setWindowTitle(tr("%1 (Flotante)").arg(title));
  float_window->setAttribute(Qt::WA_DeleteOnClose); 
  float_window->resize(content_widget->sizeHint().expandedTo(QSize(600, 400)));

  QVBoxLayout *layout = new QVBoxLayout(float_window);
  layout->setContentsMargins(5, 5, 5, 5);
  
  tab_widget_->removeTab(index);
  layout->addWidget(content_widget);
  content_widget->show();

  connect(float_window, &QDialog::destroyed, this, [this, content_widget, title, index]() {
    if (tab_widget_ && content_widget) {
      int target_index = (index <= tab_widget_->count()) ? index : tab_widget_->count();
      tab_widget_->insertTab(target_index, content_widget, title);
      tab_widget_->setCurrentIndex(target_index);
    }
  });

  float_window->show();
}