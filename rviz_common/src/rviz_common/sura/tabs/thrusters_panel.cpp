#include "thrusters_panel.hpp"
#include "rviz_common/visualization_manager.hpp"
#include "rviz_common/ros_integration/ros_node_abstraction_iface.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QScrollArea>
#include "../components/sura_button.hpp" 
#include "std_msgs/msg/float64_multi_array.hpp"
#include <QTimer>
#include <QResizeEvent>

ThrustersPanel::ThrustersPanel(rviz_common::VisualizationManager * manager, QWidget *parent)
  : QWidget(parent), manager_(manager), is_armed_(false), current_columns_(-1)
{
  QString robot_name = manager_->getRobotConfig().robot_name;

  auto ros_node = manager_->getRosNodeAbstraction().lock()->get_raw_node();
  thruster_pub_ = ros_node->create_publisher<std_msgs::msg::Float64MultiArray>(
    tr("/%1/controller/thruster_test_controller/commands").arg(robot_name).toStdString(), 10);
  
  std::string service_name = "/" + robot_name.toStdString() + "/controller/controller_manager/switch_controller";
  switch_controller_client_ = ros_node->create_client<controller_manager_msgs::srv::SwitchController>(service_name);
  
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  QLabel *title = new QLabel(tr("Thrusters control panel - %1").arg(robot_name), this);
  title->setAlignment(Qt::AlignCenter);
  QFont font = title->font();
  font.setBold(true);
  font.setPointSize(30);
  title->setFont(font);
  main_layout->addWidget(title);

  scroll_widget_ = new QWidget(this);
  thrusters_grid_ = new QGridLayout(scroll_widget_);
  thrusters_grid_->setSpacing(15);
  thrusters_grid_->setContentsMargins(5, 5, 5, 5);
  scroll_widget_->setLayout(thrusters_grid_);

  scroll_area_ = new QScrollArea(this);
  scroll_area_->setWidget(scroll_widget_);
  scroll_area_->setWidgetResizable(true); 
  scroll_area_->setFrameShape(QFrame::NoFrame); 
  scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  main_layout->addWidget(scroll_area_);

  QGroupBox *box = new QGroupBox(this);
  QGridLayout *grid_buttons = new QGridLayout(box);

  btn_save_ = new SuraButton(SuraButton::Role::Success, tr("Save"), this);
  btn_save_->setCheckable(true);
  grid_buttons->addWidget(btn_save_, 0, 0);
  
  btn_arm_ = new SuraButton(SuraButton::Role::Warning, tr("Activate"), this);
  btn_arm_->setCheckable(true);
  grid_buttons->addWidget(btn_arm_, 0, 1);
  connect(btn_arm_, &SuraButton::clicked, this, &ThrustersPanel::onArmButtonClicked);
  
  box->setLayout(grid_buttons);
  main_layout->addWidget(box);
  setLayout(main_layout);

  panel_timer_ = new QTimer(this);
  connect(panel_timer_, &QTimer::timeout, this, [this]() {
    auto message = std_msgs::msg::Float64MultiArray();
    for (auto* thruster : thruster_list_) {
      if (thruster && thruster->isRunning()) {
        message.data.push_back(thruster->getValue());
      } else {
        message.data.push_back(0.0);
      }
    }
    thruster_pub_->publish(message);
  });
}

Thruster* ThrustersPanel::addThruster(const QString &name){
  if (thruster_map_.contains(name)) {
    return thruster_map_[name]; 
  }

  Thruster *new_thruster = new Thruster(name, scroll_widget_);
  new_thruster->setMinimumWidth(220);
  new_thruster->setMaximumWidth(350);

  thruster_map_.insert(name, new_thruster);
  thruster_list_.append(new_thruster); 

  connect(new_thruster, &Thruster::runningChanged, this, [this, name](bool armed) {
      if (armed) {
        RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Thruster %s running", name.toStdString().c_str());
      } else {
        RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Thruster %s stopped.", name.toStdString().c_str());
      }
    });

  current_columns_ = -1;
  return new_thruster;
}

void ThrustersPanel::onArmButtonClicked()
{
  is_armed_ = btn_arm_->isChecked(); 
  updateArmed();
}

void ThrustersPanel::updateArmed()
{
  if (!switch_controller_client_->wait_for_service(std::chrono::seconds(1))) {
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "Error! The service controller_manager is not available.");
    return;
  }

  auto request = std::make_shared<controller_manager_msgs::srv::SwitchController::Request>();
  request->strictness = 1;

  if (is_armed_) {
    request->activate_controllers.push_back("thruster_test_controller");
    btn_arm_->setRole(SuraButton::Role::Danger);
    btn_arm_->setText(tr("Deactivate"));
    panel_timer_->start(100);
    for (auto* thruster : thruster_list_) {
      if (thruster) thruster->activate();
    }
  } else {
    request->deactivate_controllers.push_back("thruster_test_controller");
    btn_arm_->setRole(SuraButton::Role::Warning);
    btn_arm_->setText(tr("Activate"));
    panel_timer_->stop();
    for (auto* thruster : thruster_list_) {
      if (thruster) thruster->deactivate();
    }

    auto message = std_msgs::msg::Float64MultiArray();
    for (int i = 0; i < thruster_list_.size(); ++i) {
       message.data.push_back(0.0);
    }
    thruster_pub_->publish(message);
  }

  switch_controller_client_->async_send_request(
    request, [this](rclcpp::Client<controller_manager_msgs::srv::SwitchController>::SharedFuture future) {
      auto response = future.get();
      if (response->ok) {
        RCLCPP_INFO(rclcpp::get_logger("rviz2"), "SwitchController ejecutado con éxito.");
      } else {
        RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "El controller_manager rechazó el cambio de estado.");
      }
    });
}

void ThrustersPanel::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  
  int num_thrusters = static_cast<int>(thruster_list_.size());
  if (num_thrusters == 0) return;

  int available_width = event->size().width() - 50;
  if (available_width < 0) available_width = 0;
  
  int thruster_width = 250; 
  int cols = available_width / thruster_width;
  if (cols < 1) cols = 1;                  
  if (cols > num_thrusters) cols = num_thrusters;
  
  if (cols == current_columns_) return;
  current_columns_ = cols;
  
  for (int i = 0; i < num_thrusters; ++i) {
    thrusters_grid_->removeWidget(thruster_list_[i]); 
    
    int row = i / cols;
    int col = i % cols;
    
    thrusters_grid_->addWidget(thruster_list_[i], row, col); 
  }

  if (scroll_area_->widget()) {
    scroll_area_->widget()->adjustSize();
  }
}