#include "thruster.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame> // Cambiado QGroupBox por QFrame
#include <QTimer>
#include "sura_button.hpp"

const double SCALE = 100.0;

Thruster::Thruster(const QString &name, QWidget *parent)
  : QWidget(parent)
{
  QString thurtle_name = name;
  thurtle_name.replace('_', ' ');
  // Layout principal del Widget (Ajustado a márgenes 0 para que no meta espacio extra fuera de la tarjeta)
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(0, 0, 0, 0);

  // Contenedor tipo "Tarjeta" (Idéntico al de la clase Sensor)
  QFrame *card_frame = new QFrame(this);
  card_frame->setObjectName("card_frame");
  card_frame->setFrameShape(QFrame::StyledPanel);
  card_frame->setFixedSize(220, 460); // Tamaño fijo ideal para albergar el slider vertical y botones
  card_frame->setStyleSheet(
    "QFrame#card_frame {"
    "  background-color: #ffffff;"
    "  border: 1px solid #dcdde1;"
    "  border-radius: 8px;"
    "}"
  );

  QVBoxLayout *card_layout = new QVBoxLayout(card_frame);
  card_layout->setSpacing(10);
  card_layout->setContentsMargins(12, 12, 12, 12);

  // --- CABECERA: LED + Nombre ---
  QHBoxLayout *header_layout = new QHBoxLayout();
  
  // LED indicador (un QLabel redondo de 12x12)
  status_led_ = new QLabel(card_frame);
  status_led_->setFixedSize(12, 12);

  // Nombre del motor
  QLabel *title = new QLabel(thurtle_name, card_frame);
  
  QFont name_font = title->font();
  name_font.setBold(true);
  name_font.setPointSize(11);
  title->setFont(name_font);
  title->setStyleSheet("color: #2f3640;");
  title->setWordWrap(true);

  header_layout->addWidget(status_led_);
  header_layout->addWidget(title);
  header_layout->addStretch(); // Empuja los elementos a la izquierda para mantener la simetría con Sensor
  card_layout->addLayout(header_layout);

  // --- CONTENIDO DEL MOTOR ---
  thruster_min_spin_ = new QDoubleSpinBox(card_frame);
  thruster_min_spin_->setRange(-10.0, 10.0);
  thruster_min_spin_->setSingleStep(0.1);
  thruster_min_spin_->setValue(-2.0);

  thruster_max_spin_ = new QDoubleSpinBox(card_frame);
  thruster_max_spin_->setRange(-10.0, 10.0);
  thruster_max_spin_->setSingleStep(0.1);
  thruster_max_spin_->setValue(2.0); 

  thruster_slider_ = new QSlider(Qt::Vertical, card_frame);
  thruster_slider_->setRange(static_cast<int>(thruster_min_spin_->value() * SCALE),
                             static_cast<int>(thruster_max_spin_->value() * SCALE));
  thruster_slider_->setValue(0);
  thruster_slider_->setTickPosition(QSlider::TicksBothSides);
  thruster_slider_->setTickInterval(static_cast<int>(0.5 * SCALE)); 
  thruster_slider_->setMinimumHeight(150); 
  thruster_slider_->setMaximumHeight(200); // Ajustado para que encaje perfectamente en el alto de la tarjeta
  thruster_slider_->setFixedWidth(30);     

  thruster_value_spin_ = new QDoubleSpinBox(card_frame);
  thruster_value_spin_->setRange(thruster_min_spin_->value(), thruster_max_spin_->value());
  thruster_value_spin_->setSingleStep(0.05);
  thruster_value_spin_->setValue(0.0);

  thruster_invert_check_ = new QCheckBox(tr("Invertir sentido"), card_frame);
  thruster_invert_check_->setStyleSheet("color: #2f3640; font-size: 11px;");

  // Conexiones de lógica (sin cambios)
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

  card_layout->addWidget(thruster_slider_, 1, Qt::AlignHCenter);
  card_layout->addWidget(thruster_value_spin_, 0, Qt::AlignHCenter);
  card_layout->addSpacing(5);

  // Formulario de límites
  QFormLayout *form_layout = new QFormLayout();
  form_layout->setLabelAlignment(Qt::AlignLeft);
  form_layout->setFormAlignment(Qt::AlignHCenter);
  form_layout->setSpacing(6);
  
  QLabel *lbl_max = new QLabel(tr("Límite Máx:"), card_frame);
  lbl_max->setStyleSheet("color: #718093; font-size: 11px; font-weight: bold;");
  
  QLabel *lbl_min = new QLabel(tr("Límite Mín:"), card_frame);
  lbl_min->setStyleSheet("color: #718093; font-size: 11px; font-weight: bold;");

  form_layout->addRow(lbl_max, thruster_max_spin_);
  form_layout->addRow(lbl_min, thruster_min_spin_);
  form_layout->addRow(thruster_invert_check_);

  card_layout->addLayout(form_layout);

  // Botón SuraButton (Manteniendo su clase, rol y comportamiento intactos)
  run_button_ = new SuraButton(SuraButton::Role::Run, tr("RUN"), card_frame);
  run_button_->setCheckable(true);
  run_button_->setEnabled(false);
  card_layout->addWidget(run_button_);

  connect(run_button_, &QPushButton::clicked, this, &Thruster::onRunButtonClicked);

  updateLedStyle();

  main_layout->addWidget(card_frame, 0, Qt::AlignHCenter | Qt::AlignTop);
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
    // LED Verde idéntico al de la clase Sensor
    status_led_->setStyleSheet("background-color: #2ecc71; border-radius: 6px; border: none;");
    run_button_->setRole(SuraButton::Role::Danger);
    run_button_->setText(tr("STOP"));
  } else {
    // LED Rojo idéntico al de la clase Sensor
    status_led_->setStyleSheet("background-color: #e74c3c; border-radius: 6px; border: none;");
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
    return -1 * thruster_value_spin_->value();
  }
  return thruster_value_spin_->value();
}