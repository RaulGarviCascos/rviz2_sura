#ifndef Settings_PANEL_HPP
#define Settings_PANEL_HPP

#include <QWidget>
#include <QScrollArea> 
#include <QGridLayout>
#include <QLineEdit>
#include "../components/sura_button.hpp"
#include <QLabel>
#include <QIcon>
namespace rviz_common { class VisualizationFrame; }

class SettingsPanel : public QWidget
{
  Q_OBJECT

public:
explicit SettingsPanel(rviz_common::VisualizationFrame *frame_, QWidget *parent = nullptr);
~SettingsPanel() override = default;


private:
  rviz_common::VisualizationFrame * frame_;
  QGridLayout *settings_grid_;
  QScrollArea *scroll_area_;
  QLineEdit *description_file_;
  QLineEdit *input_name_;
  QLineEdit *ip_robot_;
  QLineEdit *user_name_;
  QLineEdit *input_pass_;
  SuraButton *btn_save_;
  SuraButton *get_file_button_;
  void onSaveClicked();
  void onSelectWorkspace();
  void onGetFileClicked();
  QLabel *title_label_;
  QIcon *closed_eye_icon;
  QIcon *opened_eye_icon;
  

// protected:
//   void resizeEvent(QResizeEvent *event) override;

};

#endif // Settings_PANEL_HPP