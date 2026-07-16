#ifndef RVIZ_COMMON__WELCOME_DIALOG_HPP_
#define RVIZ_COMMON__WELCOME_DIALOG_HPP_

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPixmap>
#include "sura/components/robot_config.hpp"

namespace rviz_common
{
class VisualizationManager;
class WelcomeDialog : public QDialog
{
  Q_OBJECT

public:
  explicit WelcomeDialog(QWidget * parent = nullptr, VisualizationManager * manager = nullptr);
  RobotConfig getRobotConfig(bool accepted) const;
private:
  QLabel * lbl_robot_name_;
  QDialogButtonBox * button_box_;
  QVBoxLayout * layout_;
  QLineEdit * robot_name_input_;
  QLineEdit * user_name_input_;
  QLineEdit * robot_ip_input_;
  QLineEdit * user_pass_input_;
  VisualizationManager * manager_;
  QString  robot_name_;
  QString  user_name_;
  QString  robot_ip_;
  QString  user_pass_;

};

}  

#endif  // RVIZ_COMMON__WELCOME_DIALOG_HPP_