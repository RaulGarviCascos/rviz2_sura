#include "sura_button.hpp"
#include <QStyle>

SuraButton::SuraButton(Role role, const QString &text, QWidget *parent)
  : QPushButton(text, parent), current_role_(role)
{
  updateStyle();
}

void SuraButton::setRole(Role role)
{
  current_role_ = role;
  updateStyle();
}

void SuraButton::updateStyle()
{
  QString normal_color;
  QString hover_color;
  QString pressed_color;
  QString color;
  QStyle::StandardPixmap icon_pixmap = QStyle::SP_CustomBase;

  switch (current_role_) {
    case Role::Success: 
      normal_color  = "#2ecc71"; hover_color   = "#27ae60"; pressed_color = "#1e8449";
      icon_pixmap   = QStyle::SP_DialogSaveButton;
      color = "white";
      break;

    case Role::Warning: 
      normal_color  = "#e67e22"; hover_color   = "#d35400"; pressed_color = "#b04a00";
      icon_pixmap   = QStyle::SP_MessageBoxWarning;
      color = "white";
      break;

    case Role::Danger:  
      normal_color  = "#e74c3c"; hover_color   = "#c0392b"; pressed_color = "#962d22";
      icon_pixmap   = QStyle::SP_DialogCancelButton;
      color = "white";
      break;
    case Role::Run:  
      normal_color  = "#2cddc5"; hover_color   = "#22af9c"; pressed_color = "#167064";
      icon_pixmap   = QStyle::SP_MediaPlay;
      color = "black";
      break;

    case Role::Default: 
    default:
      normal_color  = "#3498db"; hover_color   = "#2980b9"; pressed_color = "#1c638e";
      icon_pixmap   = QStyle::SP_ArrowRight;
      color = "white";
      break;
  }

  // Asignamos el icono nativo correspondiente
  if (icon_pixmap != QStyle::SP_CustomBase) {
    setIcon(style()->standardIcon(icon_pixmap));
  }

  // Aplicamos la hoja de estilo usando las variables de color inyectadas
  setStyleSheet(
    QString(
      "QPushButton {"
      "    background-color: %1;"
      "    color: %4;"
      "    font-weight: bold;"
      "    border: none;"
      "    border-radius: 4px;"
      "    padding: 6px 14px;"
      "    min-width: 90px;"
      "}"
      "QPushButton:hover {"
      "    background-color: %2;"
      "}"
      "QPushButton:pressed {"
      "    background-color: %3;"
      "}"
      "QPushButton:disabled {"
      "    background-color: #bdc3c7;"
      "    color: #7f8c8d;"
      "}"
    ).arg(normal_color, hover_color, pressed_color,color)
  );
}

/// how to use it
// 1. Botón de Guardar (Verde con disquete)
// SuraButton *btn_guardar = new SuraButton(SuraButton::Role::Success, tr("Guardar"), this);
// layout->addWidget(btn_guardar);

// // 2. Botón de Armar Motores (Naranja con señal de advertencia)
// SuraButton *btn_armar = new SuraButton(SuraButton::Role::Warning, tr("ARM MOTORS"), this);
// layout->addWidget(btn_armar);

// // 3. Botón de Parada Crítica (Rojo con aspa de cancelación)
// SuraButton *btn_stop = new SuraButton(SuraButton::Role::Danger, tr("EMERGENCY STOP"), this);
// layout->addWidget(btn_stop);

// // 4. Botón normal (Azul con flecha)
// SuraButton *btn_next = new SuraButton(SuraButton::Role::Default, tr("Siguiente"), this);
// layout->addWidget(btn_next);