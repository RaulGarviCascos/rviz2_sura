#pragma once

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMap>
#include <rclcpp/rclcpp.hpp>

#include "sura_button.hpp"
#include "sura_controller_info.hpp"

class ControllerCardWidget : public QWidget
{
  Q_OBJECT

public:
  explicit ControllerCardWidget(
    const ControllerInfo & info, 
    rclcpp::Node::SharedPtr ros_node);

  void updateState(const QString & state);
  void setLoading(bool loading);

signals:
  void toggleRequested(const QString & controller_name, bool enable);
  

private slots:
  void onToggleButtonClicked();
  void onApplyConfigClicked();
  void onToggleExpandClicked();

private:
  void buildUI();
  void updateLedStyle(bool active);
  void updateButtonStyle();
  ControllerInfo info_;
  rclcpp::Node::SharedPtr ros_node_;
  bool is_enabled_{false};
  bool is_loading_{false};

  // UI Elements
  QFrame * card_frame_{nullptr};
  QLabel * led_indicator_{nullptr};
  QLabel * name_label_{nullptr};
  QWidget * info_container_{nullptr};
  QVBoxLayout * info_layout_{nullptr};
  
  // Usamos SuraButton en lugar de QPushButton
  SuraButton * toggle_button_{nullptr};
  SuraButton * apply_button_{nullptr};
  QProgressBar * spinner_{nullptr};

  QToolButton * expand_button_{nullptr};
  QFrame * divider2_{nullptr};
  bool is_expanded_{true};

  QMap<QString, QLineEdit *> param_inputs_;
};