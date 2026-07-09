#include "./welcome_dialog.hpp"
#include <ament_index_cpp/get_package_share_directory.hpp>
#include "rviz_common/visualization_manager.hpp"

namespace rviz_common
{

WelcomeDialog::WelcomeDialog(QWidget * parent, VisualizationManager * manager)
: QDialog(parent), manager_(manager)
{
  robot_name_ = manager_->getRobotName();
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
  std::string show_text = "Enter a name for the robot:";
  if(!robot_name_.isEmpty()){
    show_text = "The current robot name is: [" + robot_name_.toStdString() + "]. Do you want to change it?";
  }
  welcome_label_ = new QLabel(tr(show_text.c_str()), this);
  welcome_label_->setAlignment(Qt::AlignLeft);

  text_input_ = new QLineEdit(this);
  text_input_->setPlaceholderText(tr("Robot name..."));
  button_box_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

  connect(button_box_, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(button_box_, &QDialogButtonBox::rejected, this, &QDialog::reject);


  layout_ = new QVBoxLayout(this);
  layout_->addWidget(image_label);    
  layout_->addSpacing(15);
  layout_->addWidget(welcome_label_);
  layout_->addWidget(text_input_);
  layout_->addSpacing(15);
  layout_->addWidget(button_box_);

  setLayout(layout_);
}

QString WelcomeDialog::getRobotName(bool accepted) const
{
  if (!accepted && !robot_name_.isEmpty()) {
    return robot_name_;
  }else if(accepted && text_input_->text().isEmpty()){
    return robot_name_;
  }
  return text_input_->text();
}

}