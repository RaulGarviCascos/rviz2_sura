#ifndef RVIZ_COMMON__WELCOME_DIALOG_HPP_
#define RVIZ_COMMON__WELCOME_DIALOG_HPP_

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPixmap>
namespace rviz_common
{

class WelcomeDialog : public QDialog
{
  Q_OBJECT

public:
  explicit WelcomeDialog(QWidget * parent = nullptr);
  virtual ~WelcomeDialog() = default;
  QString getRobotName() const;
private:
  QLabel * welcome_label_;
  QDialogButtonBox * button_box_;
  QVBoxLayout * layout_;
  QLineEdit * text_input_;
};

}  

#endif  // RVIZ_COMMON__WELCOME_DIALOG_HPP_