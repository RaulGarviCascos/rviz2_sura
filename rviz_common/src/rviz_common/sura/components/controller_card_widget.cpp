#include "controller_card_widget.hpp"
#include <rcl_interfaces/srv/set_parameters.hpp>
#include <QMetaObject>
#include <QToolButton>

namespace{
  
  rclcpp::ParameterValue parseValueFromText(const QString & text)
  {
    QString trimmed = text.trimmed();
    trimmed.replace(',', '.');
    
    // 1. Detección de Booleanos
    if (trimmed.compare("true", Qt::CaseInsensitive) == 0) {
      return rclcpp::ParameterValue(true);
    }
    if (trimmed.compare("false", Qt::CaseInsensitive) == 0) {
      return rclcpp::ParameterValue(false);
    }

    // 2. Detección de Enteros
    bool is_int = false;
    int int_val = trimmed.toInt(&is_int);

    // 3. Detección de Flotantes (Double)
    bool is_double = false;
    double double_val = trimmed.toDouble(&is_double);

    // Si contiene punto decimal o notación científica, priorizamos double sobre int
    if (is_double ) {
      RCLCPP_INFO(rclcpp::get_logger("controller_card_widget"), "Es double: %f", double_val);
      return rclcpp::ParameterValue(double_val);
    }

    if (is_int) {
      return rclcpp::ParameterValue(int_val);
    }

    if (is_double) {
      return rclcpp::ParameterValue(double_val);
    }
    RCLCPP_INFO(rclcpp::get_logger("controller_card_widget"), "Es un string de manual: %s", trimmed.toStdString().c_str());
    // 4. Si no coincide con números/bool, lo enviamos como String
    return rclcpp::ParameterValue(trimmed.toStdString());
  }
}

ControllerCardWidget::ControllerCardWidget(
  const ControllerInfo & info, 
  rclcpp::Node::SharedPtr ros_node)
: info_(info), ros_node_(ros_node), is_enabled_(info.state.toLower() == "active")
{
  buildUI();
}

void ControllerCardWidget::buildUI()
{
  QVBoxLayout * main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(0, 0, 0, 0);

  // Tarjeta Contenedora
  card_frame_ = new QFrame(this);
  card_frame_->setFrameShape(QFrame::StyledPanel);
  card_frame_->setMinimumWidth(220);
  card_frame_->setStyleSheet(
    "QFrame {"
    "  background-color: #ffffff;"
    "  border: 1px solid #e1e8ed;" 
    "  border-radius: 12px;"       
    "}"
  );

  QVBoxLayout * card_layout = new QVBoxLayout(card_frame_);
  card_layout->setSpacing(10);
  card_layout->setContentsMargins(14, 14, 14, 14);

  // --- CABECERA: LED + Nombre + Botón Desplegable ---
  QHBoxLayout * header_layout = new QHBoxLayout();
  header_layout->setSpacing(8);

  led_indicator_ = new QLabel(card_frame_);
  led_indicator_->setFixedSize(10, 10);
  updateLedStyle(is_enabled_);

  name_label_ = new QLabel(info_.name, card_frame_);
  QFont name_font = name_label_->font();
  name_font.setBold(true);
  name_font.setPointSize(12);
  name_label_->setFont(name_font);
  name_label_->setStyleSheet("border: none; color: #2c3e50;");
  name_label_->setWordWrap(true);

  // Botón desplegable / colapsable
  expand_button_ = new QToolButton(card_frame_);
  expand_button_->setCheckable(true);
  expand_button_->setChecked(false); // Desplegado por defecto
  expand_button_->setArrowType(Qt::RightArrow);
  expand_button_->setCursor(Qt::PointingHandCursor);
  expand_button_->setStyleSheet(
    "QToolButton {"
    "  border: none;"
    "  background: transparent;"
    "  color: #7f8c8d;"
    "}"
    "QToolButton:hover {"
    "  background-color: #f1f2f6;"
    "  border-radius: 4px;"
    "}"
  );

  header_layout->addWidget(led_indicator_);
  header_layout->addWidget(name_label_, 1);
  header_layout->addWidget(expand_button_);
  card_layout->addLayout(header_layout);

  // Línea divisoria superior
  QFrame * divider1 = new QFrame(card_frame_);
  divider1->setFrameShape(QFrame::HLine);
  divider1->setStyleSheet("background-color: #f1f2f6; max-height: 1px; border: none;");
  card_layout->addWidget(divider1);

  // --- TABLA ZEBRA DE PARÁMETROS ---
  info_container_ = new QWidget(card_frame_);
  info_container_->setStyleSheet("border: none; background: transparent;");

  info_layout_ = new QVBoxLayout(info_container_);
  info_layout_->setContentsMargins(0, 4, 0, 4);
  info_layout_->setSpacing(6);

  int row_index = 0;
  for (auto it = info_.params.begin(); it != info_.params.end(); ++it) {
    QFrame * row_frame = new QFrame(info_container_);
    row_frame->setFrameShape(QFrame::NoFrame);

    if (row_index % 2 == 0) {
      row_frame->setStyleSheet("QFrame { background-color: #d3d4d6; border-radius: 6px; }");
    } else {
      row_frame->setStyleSheet("QFrame { background-color: #ffffff; border-radius: 6px; }");
    }

    QHBoxLayout * row_layout = new QHBoxLayout(row_frame);
    row_layout->setContentsMargins(10, 6, 10, 6);
    row_layout->setSpacing(8);

    QLabel * label_key = new QLabel(it.key(), row_frame);
    label_key->setStyleSheet(
      "font-size: 11px;"
      "font-weight: bold;"
      "color: #596566;"
      "border: none;"
      "background: transparent;"
    );
    label_key->setWordWrap(true);

    QLineEdit * input_value = new QLineEdit(it.value(), row_frame);
    input_value->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    input_value->setStyleSheet(
      "QLineEdit {"
      "  font-size: 12px;"
      "  font-weight: bold;"
      "  color: #2c3e50;"
      "  border: none;"
      "  background: transparent;"
      "}"
      "QLineEdit:focus {"
      "  background-color: #ffffff;"
      "  border: 1px solid #3498db;"
      "  border-radius: 4px;"
      "}"
    );

    row_layout->addWidget(label_key, 1);
    row_layout->addWidget(input_value, 0);

    info_layout_->addWidget(row_frame);
    param_inputs_.insert(it.key(), input_value);
    row_index++;
  }

  info_layout_->addStretch(1);
  info_container_->setVisible(false); //plegado por defecto

  card_layout->addWidget(info_container_, 1);

  // Línea divisoria inferior
  divider2_ = new QFrame(card_frame_);
  divider2_->setFrameShape(QFrame::HLine);
  divider2_->setStyleSheet("background-color: #f1f2f6; max-height: 1px; border: none;");
  divider2_->setVisible(false); //plegado por defecto
  card_layout->addWidget(divider2_);

  

  // --- ZONA INFERIOR CON SURABUTTONS + SPINNER ---
  QVBoxLayout * bottom_layout = new QVBoxLayout();
  bottom_layout->setContentsMargins(0, 0, 0, 0);
  bottom_layout->setSpacing(6);

  apply_button_ = new SuraButton(SuraButton::Role::Success, tr("Apply Config"), this);
  toggle_button_ = new SuraButton(SuraButton::Role::Default, tr("ON"), this);
  updateButtonStyle();

  spinner_ = new QProgressBar(this);
  spinner_->setRange(0, 0);
  spinner_->setTextVisible(false);
  spinner_->setFixedHeight(6);
  spinner_->setStyleSheet(
    "QProgressBar {"
    "  border: none;"
    "  background-color: #f1f2f6;"
    "  border-radius: 3px;"
    "}"
    "QProgressBar::chunk {"
    "  background-color: #3498db;"
    "  border-radius: 3px;"
    "}"
  );
  spinner_->hide();

  bottom_layout->addWidget(apply_button_);
  bottom_layout->addWidget(toggle_button_);
  bottom_layout->addWidget(spinner_);

  card_layout->addLayout(bottom_layout);

  main_layout->addWidget(card_frame_);
  setLayout(main_layout);

  // Conexiones
  connect(expand_button_, &QToolButton::clicked, this, &ControllerCardWidget::onToggleExpandClicked);
  connect(toggle_button_, &SuraButton::clicked, this, &ControllerCardWidget::onToggleButtonClicked);
  connect(apply_button_, &SuraButton::clicked, this, &ControllerCardWidget::onApplyConfigClicked);
}

void ControllerCardWidget::onToggleExpandClicked()
{
  is_expanded_ = expand_button_->isChecked();

  // Oculta/Muestra el bloque central y la divisor inferior
  info_container_->setVisible(is_expanded_);
  divider2_->setVisible(is_expanded_);

  // Cambia la orientación de la flecha
  expand_button_->setArrowType(is_expanded_ ? Qt::DownArrow : Qt::RightArrow);

  // Ajusta el layout de la tarjeta y la ventana contenedora
  card_frame_->adjustSize();
  updateGeometry();
}

void ControllerCardWidget::setLoading(bool loading)
{
  is_loading_ = loading;
  if (loading) {
    toggle_button_->setEnabled(false);
    apply_button_->setEnabled(false);
    toggle_button_->setText(tr("Processing..."));
    spinner_->show();
  } else {
    toggle_button_->setEnabled(true);
    apply_button_->setEnabled(true);
    spinner_->hide();
    updateButtonStyle();
  }
}

void ControllerCardWidget::updateState(const QString & state)
{
  info_.state = state;
  is_enabled_ = (state.toLower() == "active");
  updateLedStyle(is_enabled_);
  setLoading(false);
}

void ControllerCardWidget::updateLedStyle(bool active)
{
  if (active) {
    led_indicator_->setStyleSheet("background-color: #2ecc71; border-radius: 5px; border: none;");
  } else {
    led_indicator_->setStyleSheet("background-color: #e74c3c; border-radius: 5px; border: none;");
  }
}

void ControllerCardWidget::updateButtonStyle()
{
  if (is_enabled_) {
    toggle_button_->setText("OFF");
    toggle_button_->setRole(SuraButton::Role::Danger);
  } else {
    toggle_button_->setText("ON");
    toggle_button_->setRole(SuraButton::Role::Default);    
  }
}

void ControllerCardWidget::onToggleButtonClicked()
{
  setLoading(true);
  emit toggleRequested(info_.name, !is_enabled_);
}

void ControllerCardWidget::onApplyConfigClicked()
{
  if (!ros_node_) return;

  setLoading(true);
  std::string set_param_service = "/bluerov/controller/" + info_.name.toStdString() + "/set_parameters";
  auto client = ros_node_->create_client<rcl_interfaces::srv::SetParameters>(set_param_service);

  if (!client->wait_for_service(std::chrono::milliseconds(300))) {
    RCLCPP_ERROR(ros_node_->get_logger(), "Servicio set_parameters no disponible para %s", info_.name.toStdString().c_str());
    setLoading(false);
    return;
  }

  auto req = std::make_shared<rcl_interfaces::srv::SetParameters::Request>();

  for (auto it = param_inputs_.begin(); it != param_inputs_.end(); ++it) {
    std::string param_name = it.key().toStdString();
    QString raw_text = it.value()->text();
    
    // 1. Convertimos el texto al ParameterValue con tipo correcto
    rclcpp::ParameterValue val = parseValueFromText(raw_text);
    
    // 2. Creamos el rclcpp::Parameter
    rclcpp::Parameter param(param_name, val);
    
    // 3. Lo convertimos al mensaje rcl_interfaces::msg::Parameter que espera la Request
    req->parameters.push_back(param.to_parameter_msg());
  }
  client->async_send_request(
    req, [this, client](rclcpp::Client<rcl_interfaces::srv::SetParameters>::SharedFuture future) {
      QMetaObject::invokeMethod(this, [this, future]() {
        try {
          auto res = future.get();
          RCLCPP_INFO(ros_node_->get_logger(), "Parametros actualizados correctamente en %s", info_.name.toStdString().c_str());
        } catch (const std::exception & e) {
          RCLCPP_ERROR(ros_node_->get_logger(), "Error guardando parametros: %s", e.what());
        }
        setLoading(false);
      }, Qt::QueuedConnection);
    });
}

