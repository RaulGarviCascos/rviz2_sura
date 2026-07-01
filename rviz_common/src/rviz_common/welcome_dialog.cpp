#include "./welcome_dialog.hpp"
namespace rviz_common
{

WelcomeDialog::WelcomeDialog(QWidget * parent)
: QDialog(parent)
{
  setWindowTitle(tr("Welcome to SURA"));
  setMinimumWidth(500); 

  QLabel * image_label = new QLabel(this);
  QString ruta_imagen = "rviz_common/images/welcome_image.png";
  QPixmap pixmap(ruta_imagen);

  if (pixmap.isNull()) {
    image_label->setText(tr("[Image not found: %1]").arg(ruta_imagen));
  } else {

    image_label->setPixmap(pixmap.scaled(800, 600, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }
  image_label->setAlignment(Qt::AlignCenter);

  
  welcome_label_ = new QLabel(tr("Enter a name for the robot:"), this);
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

QString WelcomeDialog::getRobotName() const
{
  return text_input_->text();
}

}