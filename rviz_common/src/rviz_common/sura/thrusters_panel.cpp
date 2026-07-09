#include "thrusters_panel.hpp"
#include "rviz_common/visualization_manager.hpp"
#include "rviz_common/ros_integration/ros_node_abstraction_iface.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include "sura_button.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include <QTimer>
#include <QResizeEvent>

ThrustersPanel::ThrustersPanel(rviz_common::VisualizationManager * manager, QWidget *parent)
  : QWidget(parent), manager_(manager)
{
  QString robot_name = manager_->getRobotName();

  auto ros_node = manager_->getRosNodeAbstraction().lock()->get_raw_node();
  thruster_pub_ = ros_node->create_publisher<std_msgs::msg::Float64MultiArray>(tr("/%1/controller/thruster_test_controller/commands").arg(robot_name).toStdString(), 10);
  
  std::string service_name = "/" + robot_name.toStdString() + "/controller/controller_manager/switch_controller";
  switch_controller_client_ = ros_node->create_client<controller_manager_msgs::srv::SwitchController>(service_name);
  
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  QLabel *title = new QLabel(tr("Thrusters control panel - %1").arg(robot_name), this);
  title->setAlignment(Qt::AlignCenter);
  QFont font = title->font();
  font.setBold(true);
  font.setPointSize(12);
  title->setFont(font);
  main_layout->addWidget(title);

  QWidget *scroll_widget = new QWidget(this);
  thrusters_grid_ = new QGridLayout(scroll_widget);
  for(size_t i = 0; i < NUM_ThrusterS; i++) {
    Thrusters[i] = new Thruster(tr("Thruster %1").arg(i + 1), scroll_widget);
    int row = i / current_columns_;
    int col = i % current_columns_;
    thrusters_grid_->addWidget(Thrusters[i], row, col);
    connect(Thrusters[i], &Thruster::runningChanged, this, [this, i](bool armed) {
      if (armed) {
        RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Thruster %ld running", i + 1);
        
      } else {
        RCLCPP_INFO(rclcpp::get_logger("rviz2"), "Thruster %ld stoped.", i + 1);
      }
    });
  }
  scroll_widget->setLayout(thrusters_grid_);

  scroll_area_ = new QScrollArea(this);
  scroll_area_->setWidget(scroll_widget);
  scroll_area_->setWidgetResizable(true); 
  scroll_area_->setFrameShape(QFrame::NoFrame); 
  scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  main_layout->addWidget(scroll_area_);

  QGroupBox *box = new QGroupBox(this);
  QGridLayout *grid_buttons = new QGridLayout(box);

  btn_save_ = new SuraButton(SuraButton::Role::Success, tr("Save"), this);
  btn_save_->setCheckable(true);
  grid_buttons->addWidget(btn_save_,0,0);
  
  btn_arm_ = new SuraButton(SuraButton::Role::Warning, tr("Activate"), this);
  btn_arm_->setCheckable(true);
  grid_buttons->addWidget(btn_arm_,0,1);

  connect(btn_arm_, &SuraButton::clicked, this, &ThrustersPanel::onArmButtonClicked);
  box->setLayout(grid_buttons);
  main_layout->addWidget(box);
  setLayout(main_layout);

  panel_timer_ = new QTimer(this);
  connect(panel_timer_, &QTimer::timeout, this, [this]() {
    auto message = std_msgs::msg::Float64MultiArray();
    for (size_t i = 0; i < NUM_ThrusterS; i++) {
      if (Thrusters[i]->isRunning()) {
        message.data.push_back(Thrusters[i]->getValue());
      } else {
        message.data.push_back(0.0);
      }
    }
    thruster_pub_->publish(message);
  });
}

void ThrustersPanel::onArmButtonClicked()
{
  is_armed_ = btn_arm_->isChecked(); 
  updateArmed();
}

void ThrustersPanel::updateArmed()
{
  if (!switch_controller_client_->wait_for_service(std::chrono::seconds(1))) {
    RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "¡Error! The service controller_manager is not available.");
    return;
  }
  auto request = std::make_shared<controller_manager_msgs::srv::SwitchController::Request>();
  request->strictness = 1;
  if (is_armed_) {
    request->activate_controllers.push_back("thruster_test_controller");
    btn_arm_->setRole(SuraButton::Role::Danger);
    btn_arm_->setText(tr("Deactivate"));
    panel_timer_->start(100);
    for(size_t i=0;i<NUM_ThrusterS;i++){
      Thrusters[i]->activate();
    }
  } else {
    request->deactivate_controllers.push_back("thruster_test_controller");
    btn_arm_->setRole(SuraButton::Role::Warning);
    btn_arm_->setText(tr("Activate"));
    panel_timer_->stop();
    for(size_t i=0;i<NUM_ThrusterS;i++){
      Thrusters[i]->deactivate();
    }

    //emergency stop
    auto message = std_msgs::msg::Float64MultiArray();
    for (size_t i = 0; i < NUM_ThrusterS; i++) message.data.push_back(0.0);
    thruster_pub_->publish(message);
  }
  switch_controller_client_->async_send_request(request, [this](rclcpp::Client<controller_manager_msgs::srv::SwitchController>::SharedFuture future) {
    auto response = future.get();
    if (response->ok) {
      RCLCPP_INFO(rclcpp::get_logger("rviz2"), "SwitchController ejecutado con éxito en el robot.");
    } else {
      RCLCPP_ERROR(rclcpp::get_logger("rviz2"), "El controller_manager rechazó el cambio de estado.");
    }
  });
}

void ThrustersPanel::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  
  int available_width = event->size().width() - 300;
  if (available_width < 0) available_width = 0;
  
  int Thruster_width = 250; 
  
  int cols = available_width / Thruster_width;
  if (cols < 1) cols = 1;                  
  if (cols > static_cast<int>(NUM_ThrusterS)) cols = static_cast<int>(NUM_ThrusterS);
  
  if (cols == current_columns_) return;
  current_columns_ = cols;
  
  for (size_t i = 0; i < NUM_ThrusterS; i++) {
    thrusters_grid_->removeWidget(Thrusters[i]); 
    
    int row = i / cols;
    int col = i % cols;
    
    thrusters_grid_->addWidget(Thrusters[i], row, col); 
  }
  if (scroll_area_->widget()) {
    scroll_area_->widget()->adjustSize();
  }
}