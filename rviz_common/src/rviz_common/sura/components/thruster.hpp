#ifndef Thruster_HPP
#define Thruster_HPP

#include <QWidget>
#include <QSlider>
#include <QDoubleSpinBox> 
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QTimer>
#include "sura_button.hpp"

class Thruster : public QWidget
{
  Q_OBJECT

public:
  explicit Thruster(const QString &name, QWidget *parent = nullptr);
  ~Thruster() override = default;

  double getValue();
  bool isInverted() const { return thruster_invert_check_->isChecked(); }
  bool isRunning() const { return is_running_; }
  void deactivate();
  void activate();

signals:
  void runningChanged(bool armed);

private slots:
  void onRunButtonClicked();

private:
  QSlider* thruster_slider_;
  QDoubleSpinBox* thruster_value_spin_;
  QDoubleSpinBox* thruster_min_spin_;
  QDoubleSpinBox* thruster_max_spin_;
  QCheckBox* thruster_invert_check_;
  QLabel *status_led_;
  SuraButton *run_button_; 
  bool is_running_ = false;   
  void updateLedStyle();    
};

#endif // Thruster_HPP