#include "./welcome_dialog.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include "rviz_common/visualization_manager.hpp"

namespace rviz_common
{

  WelcomeDialog::WelcomeDialog(QWidget * parent, VisualizationManager * manager)
  : QDialog(parent), manager_(manager)
  {
    robot_name_ = manager_->getRobotConfig().robot_name;
    setWindowTitle(tr("Welcome to SURA"));
    setMinimumWidth(500); 

    QLabel * image_label = new QLabel(this);
    std::string package_share_dir = ament_index_cpp::get_package_share_directory("rviz_common");
    QString ruta_imagen = QString::fromStdString(package_share_dir) + "/images/welcome_image.png";
    QPixmap pixmap(ruta_imagen);

    if (pixmap.isNull()) {
      image_label->setText(tr("[Image not found: %1]").arg(ruta_imagen));
    } else {
      image_label->setPixmap(pixmap.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    image_label->setAlignment(Qt::AlignCenter);
    std::string show_text = "Robot name:";
    
    QLabel *lbl_ip = new QLabel("Ip:");
    robot_ip_input_= new QLineEdit(this);

    lbl_robot_name_ = new QLabel(tr(show_text.c_str()), this);
    lbl_robot_name_->setAlignment(Qt::AlignLeft);

    robot_name_input_ = new QLineEdit(this);
    robot_name_input_->setPlaceholderText(tr("Robot name..."));

    QLabel *lbl_user = new QLabel("Username:");
    user_name_input_ = new QLineEdit(this);
    
    if(!robot_name_.isEmpty()){
      user_name_input_->setText(tr("%1_navigator").arg(robot_name_));
    }
    QLabel *lbl_pass = new QLabel("Password:");
    user_pass_input_ = new QLineEdit(this);
    user_pass_input_->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    user_pass_input_->setPlaceholderText(tr("Enter FTP password..."));


    button_box_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    connect(button_box_, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box_, &QDialogButtonBox::rejected, this, &QDialog::reject);


    layout_ = new QVBoxLayout(this);
    layout_->addWidget(image_label);    
    layout_->addSpacing(15);
    layout_->addWidget(lbl_ip);
    layout_->addWidget(robot_ip_input_);
    layout_->addSpacing(15);
    layout_->addWidget(lbl_robot_name_);
    layout_->addWidget(robot_name_input_);
    layout_->addSpacing(15);
    layout_->addWidget(lbl_user);
    layout_->addWidget(user_name_input_);
    layout_->addSpacing(15);
    layout_->addWidget(lbl_pass);
    layout_->addWidget(user_pass_input_);
    layout_->addSpacing(15);
    layout_->addWidget(button_box_);

    setLayout(layout_);
  }

  RobotConfig WelcomeDialog::getRobotConfig(bool accepted) const
  {
    RobotConfig r_config;
    if(accepted){
      r_config.robot_name = robot_name_input_->text();
      r_config.user = (user_name_input_->text().isEmpty())?tr("%1_navigator").arg(r_config.robot_name):user_name_input_->text();
      r_config.ip = robot_ip_input_->text();
      r_config.password = user_pass_input_->text();
    }
    return r_config;
  }

}