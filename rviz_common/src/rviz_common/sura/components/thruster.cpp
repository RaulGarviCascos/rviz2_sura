#include "thruster.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTimer>
#include "sura_button.hpp"

const double SCALE = 100.0;

Thruster::Thruster(const QString &name, QWidget *parent)
  : QWidget(parent)
{
 
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(5, 5, 5, 5);

  QHBoxLayout *header_layout = new QHBoxLayout();
  header_layout->addStretch();
  QLabel *title = new QLabel(name, this);
  title->setAlignment(Qt::AlignCenter);
  title->setFont(QFont("Arial", 10, QFont::Bold));
  header_layout->addWidget(title);

  status_led_ = new QLabel(this);
  status_led_->setFixedSize(14, 14);
  header_layout->addWidget(status_led_);
  header_layout->addStretch();

  main_layout->addLayout(header_layout);

  QGroupBox *box = new QGroupBox(this);
  
  QVBoxLayout *box_layout = new QVBoxLayout(box);
  box->setAlignment(Qt::AlignCenter);

  thruster_min_spin_ = new QDoubleSpinBox(box);
  thruster_min_spin_->setRange(-10.0, 10.0);
  thruster_min_spin_->setSingleStep(0.1);
  thruster_min_spin_->setValue(-2.0);

  thruster_max_spin_ = new QDoubleSpinBox(box);
  thruster_max_spin_->setRange(-10.0, 10.0);
  thruster_max_spin_->setSingleStep(0.1);
  thruster_max_spin_->setValue(2.0); 

  thruster_slider_ = new QSlider(Qt::Vertical, box);
  thruster_slider_->setRange(static_cast<int>(thruster_min_spin_->value() * SCALE),
                             static_cast<int>(thruster_max_spin_->value() * SCALE));
  thruster_slider_->setValue(0);
  thruster_slider_->setTickPosition(QSlider::TicksBothSides);
  thruster_slider_->setTickInterval(static_cast<int>(0.5 * SCALE)); 
  thruster_slider_->setMinimumHeight(150); 
  thruster_slider_->setMaximumHeight(350); 
  thruster_slider_->setFixedWidth(30);     


  thruster_value_spin_ = new QDoubleSpinBox(box);
  thruster_value_spin_->setRange(thruster_min_spin_->value(), thruster_max_spin_->value());
  thruster_value_spin_->setSingleStep(0.05);
  thruster_value_spin_->setValue(0.0);

  thruster_invert_check_ = new QCheckBox(tr("Invertir sentido"), box);

  connect(thruster_slider_, &QSlider::valueChanged, this, [this](int value) {
    double float_val = value / SCALE;
    if (qAbs(thruster_value_spin_->value() - float_val) > 0.001) {
      thruster_value_spin_->setValue(float_val);
    }
  });

  connect(thruster_value_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
    int slider_val = static_cast<int>(value * SCALE);
    if (thruster_slider_->value() != slider_val) {
      thruster_slider_->setValue(slider_val);
    }
  });

  connect(thruster_min_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double new_min) {
    thruster_slider_->setMinimum(static_cast<int>(new_min * SCALE));
    thruster_value_spin_->setMinimum(new_min);
  });

  connect(thruster_max_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double new_max) {
    thruster_slider_->setMaximum(static_cast<int>(new_max * SCALE));
    thruster_value_spin_->setMaximum(new_max);
  });


  box_layout->addWidget(thruster_slider_, 1, Qt::AlignHCenter);
  box_layout->addWidget(thruster_value_spin_, 0, Qt::AlignHCenter);
  box_layout->addSpacing(10);

  QFormLayout *form_layout = new QFormLayout();
  form_layout->setLabelAlignment(Qt::AlignRight);
  form_layout->setFormAlignment(Qt::AlignHCenter);
  
  form_layout->addRow(tr("Límite Máx:"), thruster_max_spin_);
  form_layout->addRow(tr("Límite Mín:"), thruster_min_spin_);
  form_layout->addRow(thruster_invert_check_);

  box_layout->addLayout(form_layout);
  

  run_button_ = new SuraButton(SuraButton::Role::Run, tr("RUN"), this);
  run_button_->setCheckable(true);
  run_button_->setEnabled(false);
  box_layout->addWidget(run_button_);

  connect(run_button_, &QPushButton::clicked, this, &Thruster::onRunButtonClicked);

  updateLedStyle();
  box->setLayout(box_layout);
  main_layout->addWidget(box);
  setLayout(main_layout);
}

void Thruster::onRunButtonClicked()
{
  is_running_ = run_button_->isChecked(); 
  
  updateLedStyle();
  
  emit runningChanged(is_running_);
}

void Thruster::updateLedStyle()
{
  if (is_running_) {
    status_led_->setStyleSheet("background-color: #2ecc71; border: 1px solid #27ae60; border-radius: 7px;");
    run_button_->setRole(SuraButton::Role::Danger);
    run_button_->setText(tr("STOP"));
  } else {
    status_led_->setStyleSheet("background-color: #e74c3c; border: 1px solid #e74c3c; border-radius: 7px;");
    run_button_->setRole(SuraButton::Role::Run);
    run_button_->setText(tr("RUN"));
 
  }
}
void Thruster::deactivate()
{
  is_running_ = false;
  thruster_slider_->setValue(0);
  thruster_value_spin_->setValue(0.0);
  run_button_->setChecked(false);
  updateLedStyle();
  run_button_->setEnabled(false);
}

void Thruster::activate(){
  run_button_->setEnabled(true);
}

double Thruster::getValue(){
  if(isInverted()){
    return -1*thruster_value_spin_->value();
  }
  return thruster_value_spin_->value();
}